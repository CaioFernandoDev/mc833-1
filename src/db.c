#include "server.h"
#include "db.h"

/*
Tivemos problemas pra manipular o arquivo JSON em C, e não queríamos usar uma lib externa pra isso.
Então decidimos fazer um DB em memória pra facilitar a manipulação, e depois sincronizamos com o JSON.
A ideia é que o DB em memória seja sempre atualizado, e o JSON só seja atualizado quando necessário.
Usamos mutexes pra garantir que não haja concorrência entre as threads.
*/
Movie LOCAL_DB[MAX_MOVIES];

int movie_count = 0;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

// Generate a random unique ID
int generate_unique_id()
{
    int id;
    int unique;
    do
    {
        id = rand();
        unique = 1;
        for (int i = 0; i < movie_count; i++)
        {
            if (LOCAL_DB[i].id == id)
            {
                unique = 0;
                break;
            }
        }
    } while (!unique && id != 0);
    return id;
}

// Save LOCAL_DB to JSON
void save_db()
{
    FILE *file = fopen(DB_FILE, "w");
    if (!file)
        return;

    fprintf(file, "[\n");
    for (int i = 0; i < movie_count; i++)
    {
        Movie *m = &LOCAL_DB[i];
        fprintf(file, "  {\n");
        fprintf(file, "    \"id\": %d,\n", m->id);
        fprintf(file, "    \"title\": \"%s\",\n", m->title);
        fprintf(file, "    \"director\": \"%s\",\n", m->director);
        fprintf(file, "    \"year\": %d,\n", m->year);
        fprintf(file, "    \"genres\": [");
        for (int j = 0; j < m->genre_count; j++)
        {
            fprintf(file, "\"%s\"%s", m->genres[j], j < m->genre_count - 1 ? ", " : "");
        }
        fprintf(file, "]\n  }%s\n", i < movie_count - 1 ? "," : "");
    }
    fprintf(file, "]\n");
    fclose(file);
}

// Load JSON file into LOCAL_DB
void load_db()
{
    FILE *file = fopen(DB_FILE, "r");
    if (!file)
        return;

    char line[512];
    Movie temp = {0};
    movie_count = 0;

    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "\"id\""))
            sscanf(line, " \"id\": %d,", &temp.id);
        else if (strstr(line, "\"title\""))
            sscanf(line, " \"title\": \"%[^\"]\",", temp.title);
        else if (strstr(line, "\"director\""))
            sscanf(line, " \"director\": \"%[^\"]\",", temp.director);
        else if (strstr(line, "\"year\""))
            sscanf(line, " \"year\": %d,", &temp.year);
        else if (strstr(line, "\"genres\""))
        {
            temp.genre_count = 0;
            char *p = strchr(line, '[');
            if (p)
            {
                char *g = strtok(p + 1, "\"");
                while (g && temp.genre_count < MAX_GENRES)
                {
                    strcpy(temp.genres[temp.genre_count++], g);
                    g = strtok(NULL, "\"");
                    g = strtok(NULL, "\"");
                }
            }
        }
        else if (strchr(line, '}'))
        {
            LOCAL_DB[movie_count++] = temp;
            memset(&temp, 0, sizeof(Movie));
        }
    }

    fclose(file);
}

Movie *find_movie_by_id(int id)
{
    for (int i = 0; i < movie_count; i++)
        if (LOCAL_DB[i].id == id)
            return &LOCAL_DB[i];
    return NULL;
}

void register_movie(char *title, char *director, int year, char *genre_str, char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    if (movie_count >= MAX_MOVIES)
    {
        sprintf(response, "Database full.\n");
        pthread_mutex_unlock(&db_mutex);
        return;
    }

    Movie *m = &LOCAL_DB[movie_count++];
    m->id = generate_unique_id();
    strncpy(m->title, title, 99);
    strncpy(m->director, director, 99);
    m->year = year;
    m->genre_count = 0;

    char *token = strtok(genre_str, ",");
    while (token && m->genre_count < MAX_GENRES)
    {
        strncpy(m->genres[m->genre_count++], token, 29);
        token = strtok(NULL, ",");
    }

    save_db();

    sprintf(response, "Movie registered with ID: %d\n", m->id);

    pthread_mutex_unlock(&db_mutex);
}

void add_genre(int id, char *genre, char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    Movie *m = find_movie_by_id(id);
    if (!m)
    {
        sprintf(response, "Movie not found.\n");
    }
    else if (m->genre_count >= MAX_GENRES)
    {
        sprintf(response, "Maximum genres reached.\n");
    }
    else
    {
        strncpy(m->genres[m->genre_count++], genre, 49);
        save_db();
        sprintf(response, "Genre added.\n");
    }

    pthread_mutex_unlock(&db_mutex);
}

void remove_movie(int id, char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    int found = 0;
    for (int i = 0; i < movie_count; i++)
    {
        if (LOCAL_DB[i].id == id)
        {
            LOCAL_DB[i] = LOCAL_DB[movie_count - 1];
            movie_count--;
            found = 1;
            break;
        }
    }
    if (found)
    {
        save_db();
        sprintf(response, "Movie removed.\n");
    }
    else
    {
        sprintf(response, "Movie not found.\n");
    }

    pthread_mutex_unlock(&db_mutex);
}

void list_titles(char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    response[0] = '\0';
    strcat(response, "ID \t\t Title\n");
    strcat(response, "-----------------------\n");
    for (int i = 0; i < movie_count; i++)
    {
        char line[150];
        sprintf(line, "%d \t %s\n", LOCAL_DB[i].id, LOCAL_DB[i].title);
        strcat(response, line);
    }
    if (movie_count == 0)
        strcpy(response, "No movies found.\n");

    pthread_mutex_unlock(&db_mutex);
}

void list_all_movies(char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    response[0] = '\0';
    for (int i = 0; i < movie_count; i++)
    {
        char line[300] = {0};
        sprintf(line, "ID: %d\nTitle: %s\nDirector: %s\nYear: %d\nGenres: ",
                LOCAL_DB[i].id, LOCAL_DB[i].title, LOCAL_DB[i].director, LOCAL_DB[i].year);
        for (int j = 0; j < LOCAL_DB[i].genre_count; j++)
        {
            strcat(line, LOCAL_DB[i].genres[j]);
            if (j < LOCAL_DB[i].genre_count - 1)
                strcat(line, ", ");
        }
        strcat(line, "\n\n");
        strcat(response, line);
    }
    if (movie_count == 0)
        strcpy(response, "No movies found.\n");

    pthread_mutex_unlock(&db_mutex);
}

void get_movie(int id, char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    Movie *m = find_movie_by_id(id);
    if (!m)
    {
        sprintf(response, "Movie not found.\n");
    }
    else
    {
        sprintf(response, "ID: %d\nTitle: %s\nDirector: %s\nYear: %d\nGenres: ",
                m->id, m->title, m->director, m->year);
        for (int j = 0; j < m->genre_count; j++)
        {
            strcat(response, m->genres[j]);
            if (j < m->genre_count - 1)
                strcat(response, ", ");
        }
        strcat(response, "\n");
    }

    pthread_mutex_unlock(&db_mutex);
}

void filter_by_genre(char *genre, char *response)
{
    pthread_mutex_lock(&db_mutex);
    load_db();

    response[0] = '\0';
    for (int i = 0; i < movie_count; i++)
    {
        for (int j = 0; j < LOCAL_DB[i].genre_count; j++)
        {
            if (strcmp(LOCAL_DB[i].genres[j], genre) == 0)
            {
                char line[150];
                sprintf(line, "%d: %s\n", LOCAL_DB[i].id, LOCAL_DB[i].title);
                strcat(response, line);
                break;
            }
        }
    }
    if (strlen(response) == 0)
        sprintf(response, "No movies found with genre '%s'.\n", genre);

    pthread_mutex_unlock(&db_mutex);
}
