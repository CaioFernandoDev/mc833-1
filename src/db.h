#ifndef DB_H
#define DB_H

#define BUFFER_LEN 1024
#define MAX_MOVIES 100
#define MAX_GENRES 5
#define DB_FILE "movies.json"

typedef struct
{
    int id;
    char title[100];
    char director[100];
    int year;
    char genres[MAX_GENRES][50];
    int genre_count;
} Movie;

int generate_unique_id();
void save_db();
void load_db();
Movie *find_movie_by_id(int id);
void register_movie(char *title, char *director, int year, char *genre_str, char *response);
void add_genre(int id, char *genre, char *response);
void remove_movie(int id, char *response);
void list_titles(char *response);
void list_all_movies(char *response);
void get_movie(int id, char *response);
void filter_by_genre(char *genre, char *response);

#endif