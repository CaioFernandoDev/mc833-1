#include "server.h"
#include "db.h"

void *handle_client(void *arg)
{
    int client_sock = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0)
            break;

        char *cmd = strtok(buffer, "|\n");
        char response[BUFFER_SIZE * 2] = {0};

        if (strcmp(cmd, "1") == 0)
        {
            char *title = strtok(NULL, "|\n");
            char *director = strtok(NULL, "|\n");
            char *year_str = strtok(NULL, "|\n");
            char *genres = strtok(NULL, "|\n");
            if (title && director && year_str && genres)
            {
                int year = atoi(year_str);
                register_movie(title, director, year, genres, response);
            }
            else
            {
                strcpy(response, "Invalid format.\n");
            }
        }
        else if (strcmp(cmd, "2") == 0)
        {
            char *id_str = strtok(NULL, "|\n");
            char *genre = strtok(NULL, "|\n");
            if (id_str && genre)
            {
                add_genre(atoi(id_str), genre, response);
            }
            else
            {
                strcpy(response, "Invalid format.\n");
            }
        }
        else if (strcmp(cmd, "3") == 0)
        {
            char *id_str = strtok(NULL, "|\n");
            if (id_str)
            {
                remove_movie(atoi(id_str), response);
            }
            else
            {
                strcpy(response, "Invalid format.\n");
            }
        }
        else if (strcmp(cmd, "4") == 0)
        {
            list_titles(response);
        }
        else if (strcmp(cmd, "5") == 0)
        {
            list_all_movies(response);
        }
        else if (strcmp(cmd, "6") == 0)
        {
            char *id_str = strtok(NULL, "|\n");
            if (id_str)
            {
                get_movie(atoi(id_str), response);
            }
            else
            {
                strcpy(response, "Invalid format.\n");
            }
        }
        else if (strcmp(cmd, "7") == 0)
        {
            char *genre = strtok(NULL, "|\n");
            if (genre)
            {
                filter_by_genre(genre, response);
            }
            else
            {
                strcpy(response, "Invalid format.\n");
            }
        }
        else
        {
            strcpy(response, "Invalid command.\n");
        }

        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock);
    return NULL;
}

int main()
{
    srand(time(NULL));
    load_db();

    int sock_fd, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sock_fd, 5);
    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        client_sock = malloc(sizeof(int));
        *client_sock = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_len);
        pthread_create(&thread_id, NULL, handle_client, client_sock);
        pthread_detach(thread_id);
    }

    close(sock_fd);
    return 0;
}