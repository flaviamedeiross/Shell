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
#include <fcntl.h>
#include <errno.h>
//#include <readline/readline.h>
//#include <readline/history.h>

#define MAX_COMANDO 1024
#define MAX_CAMINHOS 10

char *caminhos_executaveis[MAX_CAMINHOS];
int num_caminhos = 0;

void introducao()
{
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n\t\t\t\tPROJETO SHELL - INTERPRETADOR DE COMANDOS");
    printf("\n-----------------------------------------------------------------------------------------------------------------");
    printf("\n Digite um dos comandos existentes abaixo e aperte ENTER para executar:\n");
    printf("\n     exit -> Finaliza o Shell");
    printf("\n     cd <caminho> -> Muda o diretório de trabalho");
    printf("\n     path <caminho> [<caminho> <caminho> ...] -> Define caminho(s) para busca de executáveis");
    printf("\n     ls [-l] [-a] -> Lista o conteúdo do diretório atual");
    printf("\n     cat <arquivo> - > Lê o conteúdo do arquivo no argumento e o escreve na saída padrão.");
    printf("\n-----------------------------------------------------------------------------------------------------------------\n\n");
}

void exibir_prompt()
{
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

// Função para adicionar caminho temporário
void adicionar_caminho(char *caminho) {
    if (num_caminhos < MAX_CAMINHOS) {
        caminhos_executaveis[num_caminhos++] = strdup(caminho);
    } else {
        fprintf(stderr, "Número máximo de caminhos atingido\n");
    }
}

// Função comando ls
void listar_arquivos(int mostrar_ocultos, int detalhado)
{
    struct dirent *diretorio;
    DIR *dir = opendir(".");
    if (dir == NULL)
    {
        perror("Erro ao abrir diretório");
        return;
    }

    if (detalhado)
    {
        struct stat statbuf;
        int total = 0;
        while ((diretorio = readdir(dir)) != NULL)
        {
            printf("\033[0m");
            if (!mostrar_ocultos && diretorio->d_name[0] == '.')
                continue;

            if (stat(diretorio->d_name, &statbuf) == -1)
            {
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

            // Definindo a cor verde para os nomes dos arquivos/pastas
            printf("\033[0;32m"); // código de formatação ANSI para cor verde
            printf("\t%s\n", diretorio->d_name);
        }
        printf("\033[0m");
        printf("total %d\n", total / 2);
    }
    else
    {
        while ((diretorio = readdir(dir)) != NULL)
        {
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

//funcao cat
void exibir_conteudo_arquivos(int argc, char **argv){                               //numero de argumentos, array de string q contem os argumentos, nesse caso os arquivos
    
    //ve se pelo menos um arquivo foi fornecido como argumento
    if (argc <= 1) {
        fprintf(stderr, "cat: nenhum arquivo fornecido\n");                         //foi fornecido so o nome do programa ou nenhum arquivo
        return;
    }

    FILE *arquivo;
    int caracter;                                                                   //armazena caracteres lidos do arquivo

    //itera sobre os arquivos
    for (int i = 1; i < argc; i++) {
        
        //abre o arquivo
        arquivo = fopen(argv[i], "r");                                              //abrir o arquivo no modo de leitura "r"
        if (arquivo == NULL) {
            perror("cat: Não foi possível abrir o arquivo");                        //fopen retorna NULL
            continue;
        }

        //le e imprime caractere por caractere do arquivo
        while ((caracter = fgetc(arquivo)) != EOF) {                                //le ate q fgetc retorne E0F, fim do arquivo
            putchar(caracter);                                                      //imprime caracter na tela
        }
        
        //fecha o arquivo
        fclose(arquivo); 

        printf("\n");                                                          
    }
}

void verificar_comandos(char *comando, char **args) {
    int i = 0;
    char *token = strtok(comando, " ");
    char *arquivo_saida = NULL;

    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            arquivo_saida = token;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0) {
        // Implementação do comando "cd"...
    }
    else if (strcmp(args[0], "path") == 0) {
        // Implementação do comando "path"...
    }
    else if (strcmp(args[0], "ls") == 0) {
        // Implementação do comando "ls"...
    }
    else if (strcmp(args[0], "cat") == 0) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erro ao criar processo filho");
            return;
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
            exibir_conteudo_arquivos(i, args);
            exit(EXIT_SUCCESS);
        } else { // Processo pai
            waitpid(pid, NULL, 0);
        }
    }
    else {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Erro ao criar processo filho");
            return;
        }
        if (pid == 0) { // Processo filho
            int j = 0;
            while (caminhos_executaveis[j] != NULL) {
                char caminho_programa[MAX_COMANDO];
                snprintf(caminho_programa, sizeof(caminho_programa), "%s/%s", caminhos_executaveis[j], args[0]);
                if (access(caminho_programa, X_OK) == 0) {
                    execvp(caminho_programa, args);
                    perror("Erro ao executar programa");
                    exit(EXIT_FAILURE);
                }
                j++;
            }
            fprintf(stderr, "Comando não encontrado: %s\n", args[0]);
            exit(EXIT_FAILURE);
        } else { // Processo pai
            waitpid(pid, NULL, 0);
        }
    }
}



int main(int argc, char **argv)
{
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;

    introducao();

    while (1)
    {
        printf("\n");
        exibir_prompt();
        printf(" ");
        fflush(stdout);

        ler_comando = fgets(comando, sizeof(comando), stdin);



        char *novalinha = strchr(comando, '\n');
        if (novalinha != NULL){
            *novalinha = '\0';
        }

        if (ler_comando == 0){
            printf("Comando nao encontrado");
            continue;
        }

        verificar_comandos(ler_comando, args);

    }

    return 0;
}