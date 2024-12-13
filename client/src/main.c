#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>

#define DEFAULT_CLIENT_PORT 3302
#define SERVER_PORT 3204

int client_port = DEFAULT_CLIENT_PORT;

int main (int argc, char** argv) {

    /* Pouvoir changer le port du client via le terminal */
    if(argc >1){
    client_port = atoi(argv[1]);
    }
    
    /* Initialisation du socket, connect */
    int client_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
        if (client_fd == -1) { return EXIT_FAILURE; }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr= inet_addr("127.0.0.1"),
        .sin_port = htons(SERVER_PORT)
    };

    int error = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); perror("connect ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }

    /* Si la connexion au serveur est ok, on reçoit un message de bienvenue */
    char welcome[BUFSIZ]; memset (welcome, 0, BUFSIZ);
    error = recv(client_fd, welcome, sizeof(welcome) -1, 0); perror("receive ");
        if (error == -1) { return EXIT_FAILURE; }
    printf("$%s\n", welcome);

    /* Le client entre son nom */
    char user_todolist_name[BUFSIZ]; memset (user_todolist_name, 0, BUFSIZ);
    printf("Quel est ton nom? : \n");
    fgets(user_todolist_name, BUFSIZ, stdin);
    error = send(client_fd, user_todolist_name, strlen(user_todolist_name) -1, 0); perror("send ");
        if (error == -1) { close(client_fd); return EXIT_FAILURE; }

    while(1){

        /* declaration pr select() */
        fd_set readfds;
        struct timeval tv;   

        /* effacer l'ensemble à l'avance */
        FD_ZERO(&readfds);

        /* ajouter nos descripteurs à l'ensemble */
        FD_SET(client_fd, &readfds);

        /* attendre que l'un des sockets ait des données prêtes à être reçues (délai d'attente de 0,5 seconde) */
        tv.tv_sec = 0;
        tv.tv_usec = 5000;
        int retval = select(client_fd +1, &readfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");  // une erreur s'est produite dans select()
            break;

        } else if (retval == 0) { // Select ne reçoit pas de données, on fait les actions ci dessous
            printf("\nQue veux tu faire ?\n");
            printf("1. Ajouter des taches\n");
            printf("2. Mettre a jour une tache\n");
            printf("0. Quitter\n");
            
            /* le client saisit sa reponse et envoi de la donnée au serveur */
            char choix[BUFSIZ]; memset(choix, 0, BUFSIZ); 
            fgets(choix, BUFSIZ, stdin); choix[strlen(choix)-1] =0;

            int numero = atoi(choix);

            error = send(client_fd, choix, strlen(choix), 0); perror("send ");
                if (error == -1) { close(client_fd); return EXIT_FAILURE; }

            if (numero == 1) {
                /* Ajout de taches, le client saisie la tache et envoi de données au serveur */
                char taches[BUFSIZ]; memset (taches, 0, BUFSIZ);
                printf("Definissez votre tache : \n");
                fgets(taches, BUFSIZ, stdin); 

                error = send(client_fd, taches, strlen(taches) -1, 0); perror("send ");
                    if (error == -1) { close(client_fd); return EXIT_FAILURE; }

            } else if (numero == 2) { 
                /* Mise a jour de taches */
                printf("Quelle tache veux tu mettre a jour?\n");

                // Reçoit le nombre de ligne dans le fichier
                char countline[BUFSIZ]; memset(countline, 0, BUFSIZ);
                error = recv(client_fd, countline, sizeof(countline) -1, 0); perror("receive ");     
                if (error == -1) { return EXIT_FAILURE; }
                int cline = atoi(countline);

                // Reçoit le contenu du fichier
                for (int i = 0; i < cline; i++) {
                    char mline[BUFSIZ]; memset( mline, 0, BUFSIZ);
                    error = recv(client_fd, mline, sizeof(mline) -1, 0); //perror("receive ");     
                        if (error == -1) { return EXIT_FAILURE; }
                    printf("%s\n", mline);
                }

                /* Le client fait son choix et envoi de la donnée au serveur */
                char choix1[BUFSIZ]; memset(choix1, 0, BUFSIZ); 
                fgets(choix1, BUFSIZ, stdin); choix1[strlen(choix1)-1] =0;

                printf("La tache %s a été mise a jour !\n", choix1);
                error = send(client_fd, choix1, strlen(choix1), 0); perror("send :");
                    if (error == -1) { close(client_fd); return EXIT_FAILURE; }

            } else if (numero == 0) {
                /* Quitte l'application */
                return 0;
                } 
        } else { // Select () detecte une donnée, on enclenche dinc un receive 
            printf("Voici les taches a faire :\n");

            // Reçoit le nombre de ligne dans le fichier
            char countline[BUFSIZ]; memset(countline, 0, BUFSIZ);
            error = recv(client_fd, countline, sizeof(countline) -1, 0); perror("receive ");     
            if (error == -1) { return EXIT_FAILURE; }
            int cline = atoi(countline);

            // Reçoit le contenu du fichier
            for (int i = 0; i < cline; i++) {
                char mline[BUFSIZ]; memset( mline, 0, BUFSIZ);
                    error = recv(client_fd, mline, sizeof(mline) -1, 0); // perror("receive ");     
                        if (error == -1) { return EXIT_FAILURE; }
                    printf("%s\n", mline);
            }
        }
    }
    close(client_fd); perror("close ");
    return 0;
}
