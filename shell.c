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

void introducao(int argc) {
    if (argc > 1) {
        printf("\n----------------------------------------------------------------------------------------------------\n");
        printf("\nExecutando comandos do arquivo batch.. \n");
        printf("\n----------------------------------------------------------------------------------------------------\n");
        return; // Se estiver executando a partir de um arquivo, não exibe a introdução
    } 
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n\t\t\t\tPROJETO SHELL - INTERPRETADOR DE COMANDOS");
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n Digite um dos comandos existentes abaixo e aperte ENTER para executar:\n");
    printf("\n     exit                                -> Finaliza o Shell");
    printf("\n     cd <caminho>                        -> Muda o diretório de trabalho");
    printf("\n     path <caminho>/<caminho>/<caminho>  -> Define caminho(s) para busca de executáveis");
    printf("\n     <arquivo>                           -> Executa o arquivo executavel do caminho definido no path");
    printf("\n     cat <arquivo>                       -> Lê o conteúdo do arquivo no argumento e o escreve na saída padrão.");
    printf("\n     cat arquivo.txt > arquivo-saída     -> Faz o redirecionamento de saída do comando cat");
    printf("\n     ls [-l] [-a] [-la] [-al]            -> Lista o conteúdo do diretório atual");
    printf("\n     cmd1 arg1 & cmd2 arg2 & cmd3 arg3   -> Comandos separados por um '&', os executa de forma concorrente,");
    printf("\n                                            em processos separados");
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

int verificar_comandos(char **args) {
    if (args[0] == NULL) {
        fprintf(stderr, "Comando não encontrado: %s\n", args[0]);
        return 1; // Nenhum comando digitado, continue no shell
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0); // Indica que é para sair do shell

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
    } else {
        pid_t pid = fork();
        if (pid == 0) // Processo filho
        {
            // Verifica se é possível executar o comando em algum dos caminhos definidos
            int comando_encontrado = 0;
            for (int i = 0; i < num_caminhos; i++) {
                char caminho_programa[MAX_COMANDO];
                snprintf(caminho_programa, sizeof(caminho_programa), "%s/%s", caminhos_executaveis[i], args[0]);
                if (access(caminho_programa, X_OK) == 0) {
                    execvp(caminho_programa, args); 
                    perror("Erro ao executar programa");
                    exit(1); // Indica que houve um erro
                }
            }

            // Se nenhum caminho for encontrado
            fprintf(stderr, "Comando não encontrado: %s\n", args[0]);
            exit(0);
        }
        else if (pid > 0)
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {return 1;}else {return 0;}
            } else {
                perror("Erro ao esperar processo filho");
            }
        }
        else
        {   
            perror("Erro ao criar processo");
        }
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
        printf("\n");
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

        while (arg_token != NULL) {
            args[j++] = arg_token;
            arg_token = strtok(NULL, " ");
        }
        args[j] = NULL;

        if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "cat") == 0) {
            // Comando cd deve ser tratado no processo pai
            if (verificar_comandos(args) == 0) {
                return 0; // Sair do shell
            }
        } 
        else if (strcmp(args[0], "ls") == 0) {
            // Comando cd deve ser tratado no processo pai
            if (verificar_comandos(args) == 0) {
                return 0; // Sair do shell
            }
        } 
        else {
            pid_t pid = fork();
            if (pid == 0) {
                // Processo filho executa o comando
                printf("\n");
                execvp(args[0], args);
                perror("Erro ao executar comando");
                exit(1);
            }
            wait(NULL);
        }
    }

    // Espera todos os processos filhos terminarem
    for (int i = 0; i < num_cmds; i++) {
        int status;
        wait(&status);
    }

    return 1;
}

int executar_arquivo(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    char linha[MAX_COMANDO];
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Remove a nova linha no final, se existir
        linha[strcspn(linha, "\n")] = 0;

        if (strlen(linha) > 0) {
            if (strchr(linha, '&') != NULL) {
                processar_comandos(linha);
            } else {
                printf("\n");
                exibir_prompt();
                printf(" ");
                printf("%s\n", linha); // Mostra o comando lido do arquivo
                char *arg_token = strtok(linha, " ");
                char *args[100];
                int j = 0;

                while (arg_token != NULL) {
                    args[j++] = arg_token;
                    arg_token = strtok(NULL, " ");
                }
                args[j] = NULL;

                int continuar = verificar_comandos(args);
                if (continuar == 0) {
                    printf("\nComando inválido encontrado, continuando com os próximos comandos...\n");
                }
            }
        }
    }

    fclose(arquivo);
    return 1;
}

int main(int argc, char *argv[]) {
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;

    introducao(argc);

    while (1) {
       
        if(argc > 1){
        executar_arquivo(argv[1]);
        return 0;
        
    } else {
        
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

            while (arg_token != NULL) {
                args[j++] = arg_token;
                arg_token = strtok(NULL, " ");
            }
            args[j] = NULL;

            int continuar = verificar_comandos(args);
            if (continuar == 0) {
                break; // Sair do shell
            }
        }
    }
    }

    return 0;
}