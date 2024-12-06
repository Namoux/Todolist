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
int reponse_length;
long int client_fd;
char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];
char recvname[BUFSIZ];

void* pthreadclient (void* arg) {
    client_fd = (long int) arg;
            /* lire l'heure courante */
    time_t now = time (NULL);

    /* la convertir en heure locale */
    struct tm tm_now = *localtime (&now);

    /* Creer une chaine JJ/MM/AAAA HH:MM:SS */
    strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M", &tm_now);

    char bufreponse[BUFSIZ]; memset(bufreponse, 0, BUFSIZ); 
    strcpy(bufreponse, "Bienvenue dans l'application Todolist\n");
    error = send(client_fd, bufreponse, sizeof(bufreponse) -1, 0);
        if (error == -1) { return NULL; }

    // Je reçois le prenom
    char user_todolist_name[BUFSIZ];
    memset(user_todolist_name, 0, BUFSIZ); 
    reponse_length = recv(client_fd, user_todolist_name, sizeof(user_todolist_name), 0); perror("receive ");
    if (reponse_length <= 0) {
        close(client_fd); perror("close");
        printf("Client disconnected !\n");
        return NULL;
    } 

    printf ("$%s\n", recvname);

    while (1) {

        /**
         * Open personal user todo list as a readonly file descrip
         */
        FILE* fd = fopen(user_todolist_name, "r+"); 
        if (fd != NULL) { // Si y a une sauvegarde j'envoi la liste de taches
    
        char line[BUFSIZ]; memset(line, 0, BUFSIZ);
        fseek(fd, 0, SEEK_END);
        int sizeFile = ftell(fd);
        printf("%d\n", sizeFile);
        fseek(fd,0,SEEK_SET);
    
        fread(line, sizeFile, sizeof(char), fd);
        printf("%s\n", line);
    
        error = send(client_fd, line, sizeof(line) -1, 0); perror("send ");
        if (error == -1) { return NULL; }
    
        fclose(fd);
    
        } else if (fd == NULL) {
            // Create file if it does not exist
            fd = fopen(recvname, "a+"); 
            fclose(fd);
        }


        char choixs[BUFSIZ]; memset(choixs, 0, BUFSIZ);
        error = recv(client_fd, choixs, sizeof(choixs), 0); perror("receive ");     
        if (error == -1) { return NULL; }
        printf("%s\n", choixs);

        int numero = atoi(choixs);
        if (numero == 1) {
            char taches[BUFSIZ]; memset(taches, 0, BUFSIZ);
            reponse_length = recv(client_fd, taches, sizeof(taches), 0); perror("receive "); // Je reçois les taches
            if (reponse_length <= 0) {
                close(client_fd); perror("close");
                printf("Client disconnected !\n");
                return NULL;
            }
            int countline = 0;
            countline++;
            printf ("%d - %s\n", taches);
        
            FILE* fd = fopen(recvname, "a+");
            if (fd != NULL) {
                fprintf(fd, "[%s] - %d : %s", countline, s_now, taches);
                fclose(fd);
            }
        } else if (numero == 2) {
            char mline[BUFSIZ]; memset( mline, 0, BUFSIZ);

            FILE* fd = fopen(recvname, "a+");
            fseek(fd, 0, SEEK_END);
            int sizeFile = ftell(fd);
            printf("%d\n", sizeFile);
            fseek(fd,0,SEEK_SET);
            fread(mline, sizeFile, sizeof(char), fd);
            printf("%s\n", mline);

            error = send(client_fd, mline, strlen(mline), 0); perror("send ");
                if (error == -1) { close(client_fd); return NULL; }

            char choixx[BUFSIZ]; memset(choixx, 0, BUFSIZ);
            error = recv(client_fd, choixx, sizeof(choixx) -1, 0); perror("receive ");     
                if (error == -1) { return NULL; }
            int numero1 = atoi(choixx);

        for (int i = 1; i <= numero1; i++) {
            fgets(mline, BUFSIZ, fd);
            // fseek(mline, 0, SEEK_END);
            fprintf(fd, " ✅");
        }
        fclose(fd);
    } else if (numero == 0) {
        return NULL;
    }
    }

    close(client_fd); perror("close");
}

// int choice() {
//     printf("Que veux tu faire ?\n");
//     printf("1. Ajouter des taches\n");
//     printf("2. Mettre a jour une tache\n");
//     printf("0. Quitter\n");

//     char choix[BUFSIZ]; memset(choix, 0, BUFSIZ); 
//     fgets(choix, BUFSIZ, stdin); choix[strlen(choix)-1] =0;

//     int numero = atoi(choix);
//     if (numero != 0 || strcmp(choix, "0") == 0) {
//         return numero;
//     }
    
//     if (numero == 1) {
//         char taches[BUFSIZ]; memset(taches, 0, BUFSIZ);
//         reponse_length = recv(client_fd, taches, sizeof(taches), 0); perror("receive "); // Je reçois les taches
//             if (reponse_length <= 0) {
//             close(client_fd); perror("close");
//             printf("Client disconnected !\n");
//             return EXIT_FAILURE;
//             } 
//         printf ("$%s\n", taches);

//         FILE* fd = fopen(recvname, "a+");
//             if (fd != NULL) {
//                 fprintf(fd, "[%s] : %s", s_now, taches);
//                 fclose(fd);
//             }
//     } else if (numero == 2) {
//         int countline = 0;
//         char mline[BUFSIZ]; memset( mline, 0, BUFSIZ);

//         printf("Quelle tache veux tu mettre a jour?\n");

//         FILE* fd = fopen(recvname, "a+");
//         while (fgets(mline, BUFSIZ, fd) != NULL) {
//         countline++;
//         printf("%d - %s",countline, mline);
//         }
//         char choix1[BUFSIZ]; memset(choix1, 0, BUFSIZ); 
//         fgets(choix1, BUFSIZ, stdin); choix1[strlen(choix1)-1] =0;

//         int numero1 = atoi(choix);
//         if (numero1 != 0 || strcmp(choix, "0") == 0) {
//             return numero1;
//         }

//         for (int i = 1; i <= numero1; i++) {
//             fgets(mline, BUFSIZ, fd);
//             // fseek(mline, 0, SEEK_END);
//             fprintf(fd, " ✅");
//         }
//         printf("%d - %s",countline, mline);
//         fclose(fd);
//     } else if (numero == 0) {
//         return 0;
//     }
// }


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