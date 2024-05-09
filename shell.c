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
    printf("\n    1) exit -> Finaliza o Shell");
    printf("\n    2) cd <caminho> -> Muda o diretório de trabalho");
    printf("\n    3) path <caminho> [<caminho> <caminho> ...] -> Define caminho(s) para busca de executáveis");
    printf("\n    4) ls [-l] [-a] -> Lista o conteúdo do diretório atual");
    printf("\n-----------------------------------------------------------------------------------------------------------------\n\n");
}

// Função para definir caminho(s) para busca de executáveis
int definir_caminhos(char **args)
{
    // Reinicialize o número de caminhos
    num_caminhos = 0;

    for (int j = 1; args[j] != NULL && j < MAX_CAMINHOS; j++)
    {
        caminhos_executaveis[j - 1] = args[j];
        num_caminhos++;
    }

    return num_caminhos;
}

// Função para executar programas externos
void executar_programa_externo(char *caminho_completo_executavel, char **args)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Erro ao criar processo filho");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Processo filho
        // Tenta executar o comando
        execv(caminho_completo_executavel, args);
        
        // Se execv retornar, ocorreu um erro
        perror("Erro ao executar programa");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Processo pai
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            printf("Valor retornado pelo programa: %d\n", WEXITSTATUS(status));
        }
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
            printf("\t%s  ", diretorio->d_name);
        }
        printf("\n");
    }
    printf("\033[0m");
    closedir(dir);
}

int verificar_comandos(char *comando, char **args)
{
    int i = 0;
    char *token = strtok(comando, " ");

    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            fprintf(stderr, "cd: Diretório não especificado\n");
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                perror("cd");
            }
        }
        return 2;
    }
    else if (strcmp(args[0], "path") == 0)
    {
        // Lida com o comando "path"
        num_caminhos = 0; // Reinicialize o número de caminhos
        for (int j = 1; args[j] != NULL && j < MAX_CAMINHOS; j++)
        {
            caminhos_executaveis[j - 1] = args[j];
            num_caminhos++;
        }
        return 3;
    }
    else if (strcmp(args[0], "ls") == 0)
    {
        int mostrar_ocultos = 0;
        int detalhado = 0;
        for (int j = 1; args[j] != NULL; j++)
        {
            if (strcmp(args[j], "-a") == 0)
                mostrar_ocultos = 1;
            else if (strcmp(args[j], "-l") == 0)
                detalhado = 1;
            else if (strcmp(args[j], "-la") == 0 || strcmp(args[j], "-al") == 0){
                mostrar_ocultos = 1;
                detalhado = 1;
            }
        }
        listar_arquivos(mostrar_ocultos, detalhado);
        return 4;
    }
    else
    {
        // Se não for um comando interno, assumimos que é um comando externo
        char caminho_completo_executavel[MAX_COMANDO];
        sprintf(caminho_completo_executavel, "%s/%s", caminhos_executaveis[0], args[0]); // Supondo que só há um caminho definido
        executar_programa_externo(caminho_completo_executavel, args);
        return 5;
    }

    return 1;
}

int main()
{
    char comando[MAX_COMANDO], *args[MAX_COMANDO];
    char *ler_comando;

    introducao();

    while (1)
    {
        printf("Digite um comando> ");
        fflush(stdout);

        ler_comando = fgets(comando, sizeof(comando), stdin);

        char *novalinha = strchr(comando, '\n');
        if (novalinha != NULL)
            *novalinha = '\0';

        int x = verificar_comandos(ler_comando, args);

        printf("Valor de Comando: %d\n", x);
        printf("Comando recebido: %s\n", comando);

        // Exibir os caminhos de busca de executáveis
        printf("Caminhos de busca de executáveis:\n");
        for (int i = 0; i < num_caminhos; i++)
        {
            printf("%s\n", caminhos_executaveis[i]);
        }
    }

    return 0;
}
