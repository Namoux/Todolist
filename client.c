#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include "config.h"

#define DEFAULT_CLIENT_PORT 3302

int client_fd;
int client_port = DEFAULT_CLIENT_PORT;

int main (int argc, char** argv) {

        if(argc >1){
        client_port = atoi(argv[1]);
    }

    client_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
        if (client_fd == -1) { return EXIT_FAILURE; }

    // struct sockaddr_in client_addr = {
    //     .sin_family = AF_INET,
    //     .sin_addr.s_addr = INADDR_ANY,
    //     .sin_port = htons(client_port)
    // };

    // int error = bind(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)); perror("bind ");
    //     if (error == -1) { return EXIT_FAILURE; }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr= inet_addr("127.0.0.1"),
        .sin_port = htons(SERVER_PORT)
    };

    int error = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); perror("connect ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }

    char buf_response[BUFSIZ]; memset (buf_response, 0, BUFSIZ);
    error = recv(client_fd, buf_response, sizeof(buf_response) -1, 0); perror("receive ");
        if (error == -1) { return EXIT_FAILURE; }
    printf("$%s\n", buf_response);

    char sendname[BUFSIZ]; memset (sendname, 0, BUFSIZ);
    printf("Quel est ton nom? : \n");
    fgets(sendname, BUFSIZ, stdin);
    error = send(client_fd, sendname, strlen(sendname), 0); perror("send ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }


        char line[BUFSIZ]; memset (line, 0, BUFSIZ);
        // char linecopy[BUFSIZ]; memset (linecopy, 0, BUFSIZ);

    while(1){
        fd_set readfds;
        struct timeval tv;   

        // effacer l'ensemble à l'avance
        FD_ZERO(&readfds);

        // ajouter nos descripteurs à l'ensemble
        FD_SET(client_fd, &readfds);

        // attendre que l'un des sockets ait des données prêtes à être reçues (délai d'attente de 1 seconde)
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int retval = select(client_fd +1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");  // une erreur s'est produite dans select()
            break;
        } else if (retval == 0) {
            printf("Que veux tu faire ?\n");
            printf("1. Ajouter des taches\n");
            printf("2. Mettre a jour une tache\n");
            printf("0. Quitter\n");

            char choix[BUFSIZ]; memset(choix, 0, BUFSIZ); 
            fgets(choix, BUFSIZ, stdin); choix[strlen(choix)-1] =0;

            int numero = atoi(choix);
            // if (numero != 0 || strcmp(choix, "0") == 0) {
            //     return numero;
            // }
            error = send(client_fd, choix, strlen(choix), 0); perror("send ");
                if (error == -1) { close(client_fd); return EXIT_FAILURE; }

                    if (numero == 1) {
                        char taches[BUFSIZ]; memset (taches, 0, BUFSIZ);
                        printf("Definissez votre tache : \n");
                        fgets(taches, BUFSIZ, stdin); 

                        error = send(client_fd, taches, strlen(taches), 0); perror("send ");
                            if (error == -1) { close(client_fd); return EXIT_FAILURE; }
                    } else if (numero == 2) {
                        printf("Quelle tache veux tu mettre a jour?\n");
                        char mline[BUFSIZ]; memset( mline, 0, BUFSIZ);
                        error = recv(client_fd, mline, sizeof(mline) -1, 0); perror("receive ");     
                            if (error == -1) { return EXIT_FAILURE; }
                        printf("%s\n", mline);
                            char choix1[BUFSIZ]; memset(choix1, 0, BUFSIZ); 
                            fgets(choix1, BUFSIZ, stdin); choix1[strlen(choix1)-1] =0;

                            int numero1 = atoi(choix1);
                            if (numero1 != 0 || strcmp(choix, "0") == 0) {
                                return numero1;
                            }

                        error = send(client_fd, choix1, strlen(choix1), 0); perror("send ");
                            if (error == -1) { close(client_fd); return EXIT_FAILURE; }
                    } else if (numero == 0) {
                        return 0;
                        } 
        } else {
        error = recv(client_fd, line, sizeof(line) -1, 0); perror("receive ");     
            if (error == -1) { return EXIT_FAILURE; }
        }
        printf("Voici les taches a faire :\n");
        printf("%s\n", line);
    }
    close(client_fd); perror("close ");
    return 0;
}










        // strcpy(linecopy, line);
        // strtok(linecopy, ":");
        // char* token = strtok(NULL, ":");
        // token = &(token[1]); token[strlen(token)-1]= 0;
        // char* token1 = strtok(NULL, ":");
        // token1 = &(token1[1]); token1[strlen(token1)-1]= 0;
        // printf("ê%s\n", token1);