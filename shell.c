#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


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
    printf("\n-----------------------------------------------------------------------------------------------------------------\n\n");
}

int verificar_comandos(char *comando, char **args){
    
    int i=0;
    char *token = strtok(comando, " ");

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;     

    if (args[0] == NULL) {                      // Nenhum comando foi digitado
        perror("Erro ao ler o comando!");
        return 0;       
    }

    if (strcmp(args[0], "exit") == 0) {         //Comando interno para sair do shell
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {      //Comando interno para muda o diretório de trabalho
        if (args[1] == NULL) {
            fprintf(stderr, "cd: Diretório não especificado\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 2;
    }

    /*
    else if(strcmp(args[0], "path") == 0){
        if (args[1] == NULL) {
            fprintf(stderr, "path: caminho não especificado\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("path");
            }
        }
        return 3;
    }

    */

}

/*

void executar_comandos(char *comando) {

    // 0 equivale a termino de leitura, 1 equivale a termino de escrita
    //int pipefd[2]; 

    
    pid_t pid = fork();                                             //Processo filho para executar o comando

    if (pid == 0) {                                                 //Processo filho
        
        //Executar o comando

    } else if (pid < 0) { 
        perror("Erro ao criar processo filho");
        return -1;
    } else {                                                        //Processo pai
        wait(NULL);
    }                                                               
                                                                    
    return 0;                                                       
}                                                                   

*/

int main() {                                                        
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;   

    introducao();                                           

    while (1) {                                                     //Loop infinito p/ receber comandos
        printf("Digite um comando> ");                              //Prompt do shell
        fflush(stdout);                                             //Limpa o buffer de saída

        ler_comando = fgets(comando, sizeof(comando), stdin);       //Ler o comando do usuario

        char *novalinha = strchr(comando, '\n');                    //Remover o caractere \n
        if (novalinha != NULL) *novalinha = '\0';                   //Se tiver um \n, substituir por \0


        int x = verificar_comandos(ler_comando, args);

        printf("Valor de Comando: %d\n", x);
        
        printf("Comando recebido: %s\n", comando);
    }

    return 0;
}