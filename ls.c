#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>

#define MAX_COMANDO 1024
#define MAX_CAMINHOS 10

char *caminhos_executaveis[MAX_CAMINHOS];
int num_caminhos = 0;


void listar_arquivos(int mostrar_ocultos, int detalhado) {
    struct dirent *diretorio;
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Erro ao abrir diretório");
        return;
    }

    if (detalhado) {
        struct stat statbuf;
        int total = 0;
        while ((diretorio = readdir(dir)) != NULL) {
            printf("\033[0m");
            if (!mostrar_ocultos && diretorio->d_name[0] == '.')
                continue;

            if (stat(diretorio->d_name, &statbuf) == -1) {
                perror("Erro ao obter informações do arquivo");
                continue;
            }
            total += statbuf.st_blocks;

            printf((S_ISDIR(statbuf.st_mode)) ? "d" : "-");
            printf((statbuf.st_mode & S_IRUSR) ? "r" : "-");
            printf((statbuf.st_mode & S_IWUSR) ? "w" : "-");
            printf((statbuf.st_mode & S_IXUSR) ? "x" : "-");
            printf((statbuf.st_mode & S_IRGRP) ? "r" : "-");
            printf((statbuf.st_mode & S_IWGRP) ? "w" : "-");
            printf((statbuf.st_mode & S_IXGRP) ? "x" : "-");
            printf((statbuf.st_mode & S_IROTH) ? "r" : "-");
            printf((statbuf.st_mode & S_IWOTH) ? "w" : "-");
            printf((statbuf.st_mode & S_IXOTH) ? "x " : "- ");

            printf("\t%2ld ", statbuf.st_nlink);

            struct passwd *pw = getpwuid(statbuf.st_uid);
            printf("\t%s ", pw ? pw->pw_name : "unknown");

            struct group *gr = getgrgid(statbuf.st_gid);
            printf("\t%s ", gr ? gr->gr_name : "unknown");

            printf("\t%6ld ", statbuf.st_size);

            char data[30];
            strftime(data, 30, "%b %d %H:%M", localtime(&statbuf.st_mtime));
            printf("\t%s ", data);

            printf("\033[0;32m");
            printf("\t%s\n", diretorio->d_name);
        }
        printf("\033[0m");
        printf("total %d\n", total / 2);
    } else {
        int max_len = 0;
        while ((diretorio = readdir(dir)) != NULL) {
            if (!mostrar_ocultos && diretorio->d_name[0] == '.')
                continue;
              int len = strlen(diretorio->d_name);
            if (len > max_len) {
                max_len = len;
            }
        }
        rewinddir(dir); // Volta ao início do diretório

    
    // Exibe os nomes dos arquivos com alinhamento
        int columns = 5; 
        int count = 0;
        while ((diretorio = readdir(dir)) != NULL) {
            if (!mostrar_ocultos && diretorio->d_name[0] == '.')
                continue;
            printf("\033[0;32m");
            printf("%-*s  ", max_len, diretorio->d_name);
            if (++count % columns == 0) {
                printf("\n");
            }
        }
        if (count % columns != 0) {
            printf("\n");
        }
    }
    printf("\033[0m");
    closedir(dir);
}



int main(int argc, char *argv[]){

    int mostrar_ocultos = 0;
    int detalhado = 0;
    int valido = 0;
    // Verifica se os argumentos correspondem às opções válidas
    for (int i = 1; i < argc; i++){
    if (argv[i] == NULL) {
        valido = 1;
    } else if (strcmp(argv[i], "-a") == 0) {
        mostrar_ocultos = 1;
        valido = 1;
    } else if (strcmp(argv[i], "-l") == 0) {
        detalhado = 1;
        valido = 1;
    } else if (strcmp(argv[i], "-la") == 0 || strcmp(argv[i], "-al") == 0) {
        mostrar_ocultos = 1;
        detalhado = 1;
        valido = 1;
    }
     else {
        fprintf(stderr, "Comando ls: argumentos inválidos\n");
        exit(0);
    }
    }
    
    listar_arquivos(mostrar_ocultos, detalhado);
    
    return 0;
}
