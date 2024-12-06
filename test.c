#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main () {

    char name[] = "cool";
    FILE* fd = fopen(name, "a+"); 

    if (fd != NULL) { // Si y a une sauvegarde j'envoi la liste de taches
    
    char line[BUFSIZ]; memset( line, 0, BUFSIZ);
    int countline = 0;
    // fseek(fd, 0, SEEK_END);
    // int sizeFile = ftell(fd);
    // fseek(fd,0,SEEK_SET);
    // fread(line, sizeFile, sizeof(char), fd);
    while (fgets(line, BUFSIZ, fd) != NULL) {
    countline++;
    printf("%d - %s",countline, line);
    }
    fclose(fd);
    }
    
    return 0;
}