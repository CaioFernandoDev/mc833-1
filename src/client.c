#include "client.h"

void send_command(int sock, const char *command)
{
    send(sock, command, strlen(command), 0);
    char buffer[BUFFER_SIZE] = {0};
    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    printf("\n%s\n", buffer);
}

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char input[BUFFER_SIZE];

    char address[16];

    printf("Type server IP (default: 127.0.0.1, press enter to continue):\n");

    // Gets IP address from user
    // If the user presses enter, it will use the default IP address
    if (fgets(address, sizeof(address), stdin) != NULL)
    {
        char *newline = strchr(address, '\n');
        if (newline)
            *newline = '\0'; // Remove newline character

        if (strlen(address) == 0)
        {
            strcpy(address, "127.0.0.1");
        }
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(address);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("\nConnection failed.\n\n");
        return -1;
    }

    printf("Connected.\n\n");

    while (1)
    {
        printf("Lista de Comandos\n");
        printf("1 - Cadastrar um novo filme\n");
        printf("2 - Adicionar um novo gênero a um filme\n");
        printf("3 - Remover um filme pelo identificador\n");
        printf("4 - Listar todos os títulos de filmes com seus identificadores\n");
        printf("5 - Listar informações de todos os filmes\n");
        printf("6 - Listar informações de um filme específico\n");
        printf("7 - Listar todos os filmes de um determinado gênero\n");
        printf("8 - Sair\n");

        int command;
        scanf("%d", &command);
        getchar();

        char title[100], director[100], genre[50];
        int id, year;

        switch (command)
        {
        case 1:
            printf("Title: ");
            fgets(title, sizeof(title), stdin);
            title[strcspn(title, "\n")] = 0;

            printf("Director: ");
            fgets(director, sizeof(director), stdin);
            director[strcspn(director, "\n")] = 0;

            printf("Year: ");
            scanf("%d", &year);
            getchar();

            printf("Genres (comma-separated, max %d): ", MAX_GENRES);
            fgets(genre, sizeof(genre), stdin);
            genre[strcspn(genre, "\n")] = 0;

            snprintf(input, sizeof(input), "1|%s|%s|%d|%s", title, director, year, genre);
            send_command(sock, input);
            break;
        case 2:
            printf("Movie ID: ");
            scanf("%d", &id);
            getchar();

            printf("Genre: ");
            fgets(genre, sizeof(genre), stdin);
            genre[strcspn(genre, "\n")] = 0;

            snprintf(input, sizeof(input), "2|%d|%s", id, genre);
            send_command(sock, input);
            break;
        case 3:
            printf("Movie ID: ");
            scanf("%d", &id);
            getchar();

            snprintf(input, sizeof(input), "3|%d", id);
            send_command(sock, input);
            break;
        case 4:
            send_command(sock, "4");
            break;
        case 5:
            send_command(sock, "5");
            break;
        case 6:
            printf("Movie ID: ");
            scanf("%d", &id);
            getchar();

            snprintf(input, sizeof(input), "6|%d", id);
            send_command(sock, input);
            break;
        case 7:
            printf("Genre: ");
            fgets(genre, sizeof(genre), stdin);
            genre[strcspn(genre, "\n")] = 0;

            snprintf(input, sizeof(input), "7|%s", genre);
            send_command(sock, input);
            break;
        case 8:
            close(sock);
            exit(0);
        default:
            printf("Invalid option.\n\n");
        }
    }

    return 0;
}