#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <time.h>
#include "config.h"

int nb_clients = 0;
int server_fd;
int error;

void* pthreadclient (void* arg) {
    long int client_fd = (long int) arg;
            /* lire l'heure courante */
    time_t now = time (NULL);

    /* la convertir en heure locale */
    struct tm tm_now = *localtime (&now);

    /* Creer une chaine JJ/MM/AAAA HH:MM:SS */
    char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];
    strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M", &tm_now);

    char bufreponse[BUFSIZ]; memset(bufreponse, 0, BUFSIZ);
    strcpy(bufreponse, "Bienvenue dans l'application Todolist\n");
    error = send(client_fd, bufreponse, sizeof(bufreponse) -1, 0);
        if (error == -1) { return NULL; }

        char recvname[BUFSIZ]; memset(recvname, 0, BUFSIZ);
        int reponse_length = recv(client_fd, recvname, sizeof(recvname), 0); perror("receive ");
            if (reponse_length <= 0) {
            close(client_fd); perror("close");
            printf("Client disconnected !\n");
            return NULL;
            } 
        printf ("$%s\n", recvname);

    while (1) {
        char taches[BUFSIZ]; memset(taches, 0, BUFSIZ);

        FILE* fd = fopen(recvname, "r+");
        if (fd != NULL) {
        char line[BUFSIZ]; memset( line, 0, BUFSIZ);
        fseek(fd, 0, SEEK_END);
        int sizeFile = ftell(fd);
        fseek(fd,0,SEEK_SET);
        fread(line, sizeFile, sizeof(char), fd);
        printf("%s\n", line);
        error = send(client_fd, line, sizeof(line) -1, 0); perror("send ");
        if (error == -1) { return NULL; }
        fclose(fd);
        }

        reponse_length = recv(client_fd, taches, sizeof(taches), 0); perror("receive ");
            if (reponse_length <= 0) {
            close(client_fd); perror("close");
            printf("Client disconnected !\n");
            return NULL;
            } 
        printf ("$%s\n", taches);

        fd = fopen(recvname, "a+");
            if (fd != NULL) {
                fprintf(fd, "[%s] : %s", s_now, taches);
                fclose(fd);
            }
    }
        close(client_fd); perror("close");
}

int main () {

    server_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
        if (server_fd == -1) { return EXIT_FAILURE; }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(SERVER_PORT)
    };

    int error = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); perror("bind ");
        if (error == -1) { return EXIT_FAILURE; }

    error = listen(server_fd, 0); perror("listen ");
        if (error == -1) { return EXIT_FAILURE; }
    printf("Server listen to 127.0.0.1:%d\n", SERVER_PORT);

    pthread_t thread;
    struct sockaddr_in client_addr;
    socklen_t len;
    long int client_fd;

    while(1) {
        printf("Wait for a client to connect...\n");   
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len); perror("accept ");
            if (client_fd == -1) { return EXIT_FAILURE; }
        nb_clients++;   
        printf("accept numero : %d\n", nb_clients);
        pthread_create(&thread, NULL, pthreadclient, (void*)client_fd); perror("pthread_create ");
        printf("Thread starts...\n");
    }
    return EXIT_SUCCESS;
}