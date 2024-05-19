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

void introducao() {
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n\t\t\t\tPROJETO SHELL - INTERPRETADOR DE COMANDOS");
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n Digite um dos comandos existentes abaixo e aperte ENTER para executar:\n");
    printf("\n     exit -> Finaliza o Shell");
    printf("\n     cd <caminho> -> Muda o diretório de trabalho");
    printf("\n     path <caminho>/<caminho>/<caminho>  -> Define caminho(s) para busca de executáveis");
    printf("\n     cat <arquivo> -> Lê o conteúdo do arquivo no argumento e o escreve na saída padrão.");
    printf("\n     cat arquivo.txt > arquivo-saída -> Faz o redirecionamento de saída do comando cat");
    printf("\n     ls [-l] [-a] -> Lista o conteúdo do diretório atual");
    printf("\n-----------------------------------------------------------------------------------------------------------------\n\n");
}

void exibir_prompt() {
    char hostname[1024];
    gethostname(hostname, 1024);

    char username[1024];
    getlogin_r(username, 1024);

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("\033[0;32m");
    printf("%s@%s", username, hostname);
    printf("\033[0m");
    printf(":");
    printf("\033[0;36m");
    printf("%s", cwd);
    printf("\033[0m");
    printf("$ ");
}

void adicionar_caminho(char *caminho) {
    if (num_caminhos < MAX_CAMINHOS) {
        caminhos_executaveis[num_caminhos++] = strdup(caminho);
    } else {
        fprintf(stderr, "Número máximo de caminhos atingido\n");
    }
}

void exibir_conteudo_arquivos(int argc, char **argv) {
    if (argc <= 1) {
        fprintf(stderr, "cat: nenhum arquivo fornecido\n");
        return;
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
}

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
        while ((diretorio = readdir(dir)) != NULL) {
            if (!mostrar_ocultos && diretorio->d_name[0] == '.')
                continue;
            printf("\033[0;32m");
            printf("%s\t  ", diretorio->d_name);
        }
        printf("\n");
    }
    printf("\033[0m");
    closedir(dir);
}

int verificar_comandos(char **args, char *arquivo_saida) {
    if (args[0] == NULL) {
        return 1; // Nenhum comando digitado, continue no shell
    }

    if (strcmp(args[0], "exit") == 0) {
        return 0;  // Indica que é para sair do shell

    } else if (strcmp(args[0], "cd") == 0) { 
        // se o comando for "cd" entrar em um diretorio             
        char *dir = args[1];
        
        // Se nenhum diretório é especificado ou ~ (home) é usado
        if (dir == NULL || strcmp(dir, "~") == 0) 
        {
            //retorna um ponteiro contendo informações sobre o usuário atual 
            struct passwd *pw = getpwuid(getuid());      
            
            if (pw == NULL) {
                perror("getpwuid");
                return 1;
            }
            //atribui o diretório home do usuário à variável dir
            dir = pw->pw_dir;                           
        }

        if (chdir(dir) != 0) {
            if (errno == ENOENT) {
                fprintf(stderr, "cd: Diretório '%s' não existe\n", dir);
            } else {
                perror("cd");
            }
        }

    } else if (strcmp(args[0], "path") == 0) {
        num_caminhos = 0; // Limpar os caminhos anteriores
        for (int i = 1; args[i] != NULL; i++) {
            adicionar_caminho(args[i]);
        }

    } else if (strcmp(args[0], "cat") == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erro ao criar processo filho");
            return 1;
        }
        if (pid == 0) { // Processo filho
            if (arquivo_saida != NULL) {
                int fd = open(arquivo_saida, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("Erro ao abrir arquivo de saída");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("Erro ao redirecionar saída para arquivo");
                    exit(EXIT_FAILURE);
                }
                close(fd);
            }
            int arquivos = 0;
            for (int i = 0; args[i] != NULL; i++) {
                arquivos++;
            }
            exibir_conteudo_arquivos(arquivos, args);
            
            exit(EXIT_SUCCESS);
        } else { // Processo pai
            waitpid(pid, NULL, 0);
        }

    } else if (strcmp(args[0], "ls") == 0) {
        int mostrar_ocultos = 0;
        int detalhado = 0;
        for (int j = 1; args[j] != NULL; j++) {
            if (strcmp(args[j], "-a") == 0)
                mostrar_ocultos = 1;
            else if (strcmp(args[j], "-l") == 0)
                detalhado = 1;
            else if (strcmp(args[j], "-la") == 0 || strcmp(args[j], "-al") == 0) {
                mostrar_ocultos = 1;
                detalhado = 1;
            }
        }
        listar_arquivos(mostrar_ocultos, detalhado);

    } else {
        for (int i = 0; i < num_caminhos; i++) {
            char caminho_programa[MAX_COMANDO];
            snprintf(caminho_programa, sizeof(caminho_programa), "%s/%s", caminhos_executaveis[i], args[0]);
            if (access(caminho_programa, X_OK) == 0) {
                execvp(caminho_programa, args);
                perror("Erro ao executar programa");
                exit(1); // Indique que houve um erro
            }
        }
        fprintf(stderr, "%s: Comando não encontrado\n", args[0]);
        return 1;
    }
    return 1; // Continue no shell
}

int processar_comandos(char *comando) {
    char *token;
    char *cmd_args[100]; // Assumindo que no máximo 100 comandos/argumentos são permitidos
    int num_cmds = 0;

    // Separa os comandos e argumentos
    token = strtok(comando, "&");
    while (token != NULL) {
        cmd_args[num_cmds++] = token;
        token = strtok(NULL, "&");
    }

    // Processa os comandos e argumentos
    for (int i = 0; i < num_cmds; i++) {
        if (cmd_args[i] == NULL) {
            continue; // Pode haver espaços em branco entre &
        }

        // Remove espaços em branco no começo do comando/argumento
        while (isspace((unsigned char) cmd_args[i][0])) {
            memmove(cmd_args[i], cmd_args[i] + 1, strlen(cmd_args[i]));
        }

        // Remove espaços em branco no final do comando/argumento
        while (isspace((unsigned char) cmd_args[i][strlen(cmd_args[i]) - 1])) {
            cmd_args[i][strlen(cmd_args[i]) - 1] = '\0';
        }

        // Verifica comandos e executa em background
        char *arg_token = strtok(cmd_args[i], " ");
        char *args[100]; // Assumindo que no máximo 100 argumentos são permitidos
        int j = 0;
        char *arquivo_saida = NULL;

        while (arg_token != NULL) {
            if (strcmp(arg_token, ">") == 0) {
                arg_token = strtok(NULL, " ");
                arquivo_saida = arg_token;
                break;
            }
            args[j++] = arg_token;
            arg_token = strtok(NULL, " ");
        }
        args[j] = NULL;

        if (strcmp(args[0], "cd") == 0) {
            // Comando cd deve ser tratado no processo pai
            if (verificar_comandos(args, arquivo_saida) == 0) {
                return 0; // Sair do shell
            }
        }else if(strcmp(args[0], "cat") == 0){
            verificar_comandos(args, arquivo_saida);
        
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                // Processo filho executa o comando
                execvp(args[0], args);
                perror("Erro ao executar comando");
                exit(1);
            }
        }
    }

    // Espera todos os processos filhos terminarem
    for (int i = 0; i < num_cmds; i++) {
        int status;
        wait(&status);
    }

    return 1;
}

int main() {
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;

    introducao();

    while (1) {
        printf("\n");
        exibir_prompt();
        printf(" ");
        fflush(stdout);

        ler_comando = fgets(comando, sizeof(comando), stdin);

        if (ler_comando == NULL) {
            printf("\n");
            continue;
        }

        char *novalinha = strchr(comando, '\n');
        if (novalinha != NULL)
            *novalinha = '\0';

        if (strlen(comando) == 0) {
            continue; // Comando vazio, continue no loop
        }

        // Verifica a presença de & e processa os comandos
        if (strchr(comando, '&') != NULL) {
            processar_comandos(comando);
        } else {
            char *arg_token = strtok(comando, " ");
            int j = 0;
            char *arquivo_saida = NULL;

            while (arg_token != NULL) {
                if (strcmp(arg_token, ">") == 0) {
                    arg_token = strtok(NULL, " ");
                    arquivo_saida = arg_token;
                    break;
                }
                args[j++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[j] = NULL;
            int continuar = verificar_comandos(args, arquivo_saida);
            if (continuar == 0) {
                break; // Sair do shell
            }
        }
    }

    return 0;
}