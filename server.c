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
int countline = 0;

void* pthreadclient (void* arg) {

    int client_fd = (long int) arg;

    /* lire l'heure courante */
    time_t now = time (NULL);

    /* la convertir en heure locale */
    struct tm tm_now = *localtime (&now);

    /* Creer une chaine JJ/MM/AAAA HH:MM */
    char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];
    strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M", &tm_now);

    char welcome[BUFSIZ]; memset(welcome, 0, BUFSIZ); 
    strcpy(welcome, "Bienvenue dans l'application Todolist\n");
    int error = send(client_fd, welcome, sizeof(welcome) -1, 0);perror("send");
        if (error == -1) { return NULL; }

    /* Je reçois le prenom */
    char user_todolist_name[BUFSIZ]; memset(user_todolist_name, 0, BUFSIZ); 
    int reponse_length = recv(client_fd, user_todolist_name, sizeof(user_todolist_name), 0); perror("receive ");
    if (reponse_length <= 0) {
        close(client_fd); perror("close");
        printf("Client disconnected !\n");
        nb_clients--;
        return NULL;
    } 
    printf ("l'utilisateur %s s'est connecté\n", user_todolist_name);

    while (1) {
        /**
         * Open personal user todo list as a readonly file descrip
         */
        FILE* fd = fopen(user_todolist_name, "r+"); perror("fopen r+"); 
        if (fd != NULL) { /* Si y a une sauvegarde j'envoi la liste de taches */
            char line[BUFSIZ]; memset(line, 0, BUFSIZ);
            /* Je compte le nombre de lignes dans le fichier */
            while (fgets(line, BUFSIZ, fd) != NULL) {
                countline = atoi(line);
            }

            /* J'envoie le nombre de ligne au client */
            char countline1[BUFSIZ]; memset(countline1, 0, BUFSIZ);
            sprintf(countline1, "%d", countline);
            error = send(client_fd, countline1, strlen(countline1), 0); perror("send ");
                if (error == -1) { close(client_fd); return NULL; }

            /* Je replace le curseur au debut du fichier */
            fseek(fd,0,SEEK_SET);
            
            /* On lis et decortique le fichier pour l'envoyer au client dans un format specifique */
            char mline[BUFSIZ]; memset(mline, 0, BUFSIZ);
            char idline[BUFSIZ]; memset(idline, 0, BUFSIZ);
            char btext[BUFSIZ]; memset(btext, 0, BUFSIZ);
            for(int i = 0; i < countline; i++) {
                fgets(mline, BUFSIZ, fd);
                strcpy(idline, mline);
                strtok(idline, ",");
                char *dateline = strtok(NULL, ",");
                char *textline = strtok(NULL, ",");
                char *doneline = strtok(NULL, ",");
                int done = atoi(doneline);

                if (done == 1) {
                    sprintf(btext, "%s - [%s] : %s ✅", idline, dateline, textline);
                    usleep(10000); /* Permet une meilleure synchronisation avec le recv du client */
                    error = send(client_fd, btext, strlen(btext), 0); perror("send ");
                        if (error == -1) { close(client_fd); return NULL; }
                } else {
                    sprintf(btext, "%s - [%s] : %s", idline, dateline, textline);
                    usleep(10000);
                    error = send(client_fd, btext, strlen(btext), 0); perror("send ");
                        if (error == -1) { close(client_fd); return NULL; }
                }
                }

            printf("Sa liste des taches a été envoyé\n");
            fclose(fd); perror("fclose r+");

        } else {
            /* Create file if it does not exist */
            fd = fopen(user_todolist_name, "a+"); perror("fopen a+");
            printf("Utilisateur inconnu, Nouveau client crée\n"); 
            fclose(fd); perror("fclose a+");
        }

            /* Le client est devant un menu avec un choix, on attend sa reponse */
            char choix[BUFSIZ]; memset(choix, 0, BUFSIZ);
            printf("En attente de ce que le client veut faire...\n");
            error = recv(client_fd, choix, sizeof(choix), 0); perror("receive ");     
            if (error == -1) { return NULL; }
            printf("Le client a fait le choix numero : %s\n", choix);
            int numero = atoi(choix);

        if (numero == 1) {

            /* Ajout de taches, On attend qu'il saisisse sa tache */
            char taches[BUFSIZ]; memset(taches, 0, BUFSIZ);
            printf("En attente de la tache a lister\n");
            reponse_length = recv(client_fd, taches, sizeof(taches), 0); perror("receive "); // Je reçois les taches
            if (reponse_length <= 0) {
                close(client_fd); perror("close");
                printf("Client disconnected !\n");
                nb_clients--;
                return NULL;
            }
            countline++;
            printf ("%d - %s\n",countline, taches);
        
            /* On ecrit sur le fichier la tache au format csv */
            FILE* fd = fopen(user_todolist_name, "a+");
            if (fd != NULL) {
                fprintf(fd, "%d,%s,%s,0\n", countline, s_now, taches);
                fclose(fd);
            }
        } else if (numero == 2) {

            /* Mise a jour de taches, on envoi ce que contient le fichier */
            FILE* fd = fopen(user_todolist_name, "r+"); perror("fopen r+ : ");
            char countline1[BUFSIZ]; memset(countline1, 0, BUFSIZ);
            sprintf(countline1, "%d", countline);
            error = send(client_fd, countline1, strlen(countline1), 0); perror("send ");
                if (error == -1) { close(client_fd); return NULL; }

            /* On renvoi les taches pour que le client puisse savoir quelle tache mettre a jour */
            char mline[BUFSIZ]; memset(mline, 0, BUFSIZ);
            char idline[BUFSIZ]; memset(idline, 0, BUFSIZ);
            char btext[BUFSIZ]; memset(btext, 0, BUFSIZ);
            for(int i = 0; i < countline; i++) {
                fgets(mline, BUFSIZ, fd);
                strcpy(idline, mline);
                strtok(idline, ",");
                char *dateline = strtok(NULL, ",");
                char *textline = strtok(NULL, ",");
                char *doneline = strtok(NULL, ",");
                int done = atoi(doneline);
                    if (done == 1) {
                        sprintf(btext, "%s - [%s] : %s ✅", idline, dateline, textline);
                        usleep(10000);
                        error = send(client_fd, btext, strlen(btext), 0); perror("send ");
                            if (error == -1) { close(client_fd); return NULL; }
                    } else {
                        sprintf(btext, "%s - [%s] : %s", idline, dateline, textline);
                        usleep(10000);
                        error = send(client_fd, btext, strlen(btext), 0); perror("send ");
                            if (error == -1) { close(client_fd); return NULL; }
                    }
                }

            /* On attend le choix du client sur la tache qu'il veut mettre a jour */
            char choixx[BUFSIZ]; memset(choixx, 0, BUFSIZ);
            printf("En attente de la tache a modifier\n");
            error = recv(client_fd, choixx, sizeof(choixx) -1, 0); perror("receive ");     
                if (error == -1) { return NULL; }
            int numero1 = atoi(choixx);

            /* Apres le choix du client, on place le curseur au debut et on va a la ligne demandée par le client et le met a jour */
            fseek(fd,0,SEEK_SET);

            for (int i= 0; i < (numero1); i++) {
                fgets(mline, BUFSIZ, fd);
            }

            printf("mline = %s\n", mline);
            strcpy(idline, mline);
            strtok(idline, ",");
            char *dateline = strtok(NULL, ",");
            char *textline = strtok(NULL, ",");
            char *doneline = strtok(NULL, ",");
            int done = atoi(doneline);

            fseek(fd,-strlen(mline),SEEK_CUR);
            if (done == 1) {
                strcpy(doneline, "0");
                fprintf(fd, "%s,%s,%s,%s\n", idline, dateline, textline, doneline);
            } else {
                strcpy(doneline, "1");
                fprintf(fd, "%s,%s,%s,%s\n", idline, dateline, textline, doneline);
            }
            fclose(fd);
        } else if (numero == 0) {
            /* Le client a quitté l'application, il est deconnecté du serveur */
            close(client_fd);
            printf("Client disconnected !\n");
            break;
        }
    }
    close(client_fd); perror("close");
}

int main () {

    /**
     * Initialisation du socket (socket, bind, listen)
     */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); perror("socket ");
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
    printf("Server listen in port: %d\n", SERVER_PORT);

    /* On lance un pthread pour une connexion simultanée */
    pthread_t thread;
    struct sockaddr_in client_addr;
    socklen_t len;
    while(1) {
        printf("Wait for a client to connect...\n");   
        long int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len); perror("accept ");
            if (client_fd == -1) { return EXIT_FAILURE; }
        nb_clients++;   
        printf("Clients connectés : %d\n", nb_clients);
        pthread_create(&thread, NULL, pthreadclient, (void*)client_fd); perror("pthread_create ");
        printf("Thread starts...\n");

        /* On reset le countline pour chaque nouveau client */
        countline = 0;

    }






    return EXIT_SUCCESS;
}