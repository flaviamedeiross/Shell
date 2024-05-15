#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pwd.h>


#define MAX_COMANDO 1024

void introducao()
{
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n\t\t\t\tPROJETO SHELL - INTERPRETADOR DE COMANDOS");
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n Digite um dos comandos existentes abaixo e aperte ENTER para executar:\n");
    printf("\n    1) exit -> Finaliza o Shell");
    printf("\n    2) cd <caminho> -> Muda o diretório de trabalho");
    printf("\n    3) path <caminho> [<caminho> <caminho> ...] -> Define caminho(s) para busca de executáveis");
    printf("\n-----------------------------------------------------------------------------------------------------------------\n");
}

void print_diretorio_atual()
{
    char caminho[1024];
    getcwd(caminho, sizeof(caminho));
    printf("\n$%s> ", caminho);     
}

void verificar_comandos(char *comando, char **args){
    
    int i = 0;
    char *token = strtok(comando, " ");

    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;   

    if (args[0] == NULL) {                              // Nenhum comando foi digitado
        perror("Erro ao ler o comando!");       
    }

    if (strcmp(args[0], "exit") == 0) {                 //Comando interno para sair do shell
        exit(0);
    }


    else if (strcmp(args[0], "cd") == 0) { 
        // se o comando for "cd" entrar em um diretorio             
        char *dir = args[1];
        
        // Se nenhum diretório é especificado ou ~ (home) é usado
        if (dir == NULL || strcmp(dir, "~") == 0) 
        { 
            struct passwd *pw = getpwuid(getuid());     //retorna um ponteiro contendo informações sobre o usuário atual 
            
            if (pw == NULL) {
                perror("getpwuid");
                return;
            }
            dir = pw->pw_dir;                           //atribui o diretório home do usuário à variável dir
        }

        if (chdir(dir) != 0) {
            if (errno == ENOENT) {
                fprintf(stderr, "cd: Diretório '%s' não existe\n", dir);
            } else {
                perror("cd");
            }
        }
    } 

}


int main() {                                                        
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;   

    introducao();                                           

    while (1) {                                                     //Loop infinito p/ receber comandos
        print_diretorio_atual();                                    //Prompt do shell
        fflush(stdout);                                             //Limpa o buffer de saída

        ler_comando = fgets(comando, sizeof(comando), stdin);       //Ler o comando do usuario

        char *novalinha = strchr(comando, '\n');                    //Remover o caractere \n
        if (novalinha != NULL) *novalinha = '\0';                   //Se tiver um \n, substituir por \0

        if (ler_comando == NULL || strcmp(comando, "\n") == 0) {    //Erro durante a leitura
                    perror("Erro ao ler o comando");
                    continue;                                       //Continua para o prox loop
                }

        verificar_comandos(ler_comando, args);
        
    }

    return 0;
}
