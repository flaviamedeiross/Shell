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

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "cat: nenhum arquivo fornecido\n");
        return 1;
    }

    int existe_redirecionamento = 0;
    int output_fd = -1;                 // verifica se ha a abertura do arquivo de saida
    int arg_index = 1;                  // percorre os argumentos do programa

    // verifica se ha o operador do redirecionamento de saida (>)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0) {
            // Verifica se há um argumento subsequente (argv[i + 1])
            if (i + 1 < argc) {
                output_fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd == -1) {
                    perror("Erro ao abrir arquivo de saída");
                    return 1;
                }
                existe_redirecionamento = 1;
                // ajusta argc para ignorar os argumentos após >
                argc = i;
                break;
            } else {
                fprintf(stderr, "cat: operador de redirecionamento '>' sem arquivo de destino\n");
                return 1;
            }
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Erro ao criar processo filho");
        return 1;
    }

    if (pid == 0) { // Processo filho
        if (existe_redirecionamento) {
            // dup2 redireciona a saída padrão (stdout) para o arquivo aberto
            if (dup2(output_fd, STDOUT_FILENO) == -1) {
                perror("Erro ao redirecionar saída para arquivo");
                exit(EXIT_FAILURE);
            }
            close(output_fd);
        }

        FILE *arquivo;
        int caracter;

        // Itera sobre os argumentos
        for (int i = 1; i < argc; i++) {

            arquivo = fopen(argv[i], "r");
            if (arquivo == NULL) {
                perror("cat: Não foi possível abrir o arquivo");
                continue;
            }
            // Le e imprime na tela caracter por caracter do arquivo
            while ((caracter = fgetc(arquivo)) != EOF) {
                putchar(caracter);
            }
            fclose(arquivo);
        }
        exit(EXIT_SUCCESS);

    } else { // Processo pai
        waitpid(pid, NULL, 0);
    }

    return 0;
}
