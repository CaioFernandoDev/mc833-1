#include "server.h"
#include "db.h"

Movie db[MAX_MOVIES];
int movie_count = 0;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

#define DB_FILE "movies.json"

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
            if (db[i].id == id)
            {
                unique = 0;
                break;
            }
        }
    } while (!unique);
    return id;
}

// Save DB to JSON
void save_db()
{
    FILE *file = fopen(DB_FILE, "w");
    if (!file)
        return;

    fprintf(file, "[\n");
    for (int i = 0; i < movie_count; i++)
    {
        Movie *m = &db[i];
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

// Load DB from JSON
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
            db[movie_count++] = temp;
            memset(&temp, 0, sizeof(Movie));
        }
    }

    fclose(file);
}

Movie *find_movie_by_id(int id)
{
    for (int i = 0; i < movie_count; i++)
        if (db[i].id == id)
            return &db[i];
    return NULL;
}

void register_movie(char *title, char *director, int year, char *genre_str, char *response)
{
    pthread_mutex_lock(&db_mutex);

    if (movie_count >= MAX_MOVIES)
    {
        sprintf(response, "Database full.\n");
        pthread_mutex_unlock(&db_mutex);
        return;
    }

    Movie *m = &db[movie_count++];
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
        strncpy(m->genres[m->genre_count++], genre, 29);
        save_db();
        sprintf(response, "Genre added.\n");
    }

    pthread_mutex_unlock(&db_mutex);
}

void remove_movie(int id, char *response)
{
    pthread_mutex_lock(&db_mutex);

    int found = 0;
    for (int i = 0; i < movie_count; i++)
    {
        if (db[i].id == id)
        {
            db[i] = db[movie_count - 1];
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

    response[0] = '\0';
    strcat(response, "ID \t\t Title\n");
    strcat(response, "-----------------------\n");
    for (int i = 0; i < movie_count; i++)
    {
        char line[150];
        sprintf(line, "%d \t %s\n", db[i].id, db[i].title);
        strcat(response, line);
    }
    if (movie_count == 0)
        strcpy(response, "No movies found.\n");

    pthread_mutex_unlock(&db_mutex);
}

void list_all_movies(char *response)
{
    pthread_mutex_lock(&db_mutex);

    response[0] = '\0';
    for (int i = 0; i < movie_count; i++)
    {
        char line[300] = {0};
        sprintf(line, "ID: %d\nTitle: %s\nDirector: %s\nYear: %d\nGenres: ",
                db[i].id, db[i].title, db[i].director, db[i].year);
        for (int j = 0; j < db[i].genre_count; j++)
        {
            strcat(line, db[i].genres[j]);
            if (j < db[i].genre_count - 1)
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

    response[0] = '\0';
    for (int i = 0; i < movie_count; i++)
    {
        for (int j = 0; j < db[i].genre_count; j++)
        {
            if (strcmp(db[i].genres[j], genre) == 0)
            {
                char line[150];
                sprintf(line, "%d: %s\n", db[i].id, db[i].title);
                strcat(response, line);
                break;
            }
        }
    }
    if (strlen(response) == 0)
        sprintf(response, "No movies found with genre '%s'.\n", genre);

    pthread_mutex_unlock(&db_mutex);
}
