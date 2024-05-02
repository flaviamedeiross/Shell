#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMANDO 1024

int executar_comandos(char *comando) {
  
    if (strcmp(comando, "sair") == 0) {
        exit(0);                                                    //Comando interno para sair do shell
    }
    
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

int main() {                                                        
    char comando[MAX_COMANDO];
    char *ler_comando;                                              

    while (1) {                                                     //Loop infinito p/ receber comandos
        printf("Digite um comando> ");                              //Prompt do shell
        fflush(stdout);                                             //Limpa o buffer de saída

        ler_comando = fgets(comando, sizeof(comando), stdin);       //Ler o comando do usuario

        if (ler_comando == NULL) {                                  //Erro durante a leitura
            perror("Erro ao ler o comando");
            continue;                                               //Continua para o prox loop
        }

        char *novalinha = strchr(comando, '\n');                    //Remover o caractere \n
        if (novalinha != NULL) *novalinha = '\0';                   //Se tiver um \n, substituir por \0

        printf("Comando recebido: %s\n", comando);
    }

    return 0;
}
