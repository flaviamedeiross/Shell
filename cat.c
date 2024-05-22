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

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "cat: nenhum arquivo fornecido\n");
        return 1;
    }

    FILE *arquivo;
    int caracter;

    for (int i = 1; i < argc; i++) {
        arquivo = fopen(argv[i], "r");
        if (arquivo == NULL) {
            perror("cat: Não foi possível abrir o arquivo");
            continue;
        }
        while ((caracter = fgetc(arquivo)) != EOF) {
            putchar(caracter);
        }

        fclose(arquivo);
    }
    return 0;
}