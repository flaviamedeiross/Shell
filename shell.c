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
    printf("\n     exit -> Finaliza o Shell");
    printf("\n     cd <caminho> -> Muda o diretório de trabalho");
    printf("\n     path <caminho> [<caminho> <caminho> ...] -> Define caminho(s) para busca de executáveis");
    printf("\n     cat <arquivo> - > Lê o conteúdo do arquivo no argumento e o escreve na saída padrão.");
    printf("\n     ls [-l] [-a] -> Lista o conteúdo do diretório atual");
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

//funcao cat
void exibir_conteudo_arquivos(int argc, char **argv){
    // Verifica se pelo menos um arquivo foi fornecido como argumento
    if (argc <= 1) {
        fprintf(stderr, "cat: nenhum arquivo fornecido\n");
        return;
    }

    FILE *arquivo;
    int caracter;

    // Itera sobre os argumentos (arquivos)
    for (int i = 1; i < argc; i++) {
        // Abre o arquivo
        arquivo = fopen(argv[i], "r");
        if (arquivo == NULL) {
            perror("cat: Não foi possível abrir o arquivo");
            continue;
        }

        // Lê e exibe o conteúdo do arquivo caractere por caractere
        while ((caracter = fgetc(arquivo)) != EOF) {
            putchar(caracter);
        }

        // Fecha o arquivo
        fclose(arquivo);
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

    if (args[0] == NULL) {
        return 1; // Nenhum comando digitado, continue no shell
    }

    if (strcmp(args[0], "exit") == 0)
    {
        return 0;  // Indica que é para sair do shell
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
    }
    else if (strcmp(args[0], "path") == 0) {
        // Se o comando for "path", definir caminhos temporários
        num_caminhos = 0; // Limpar os caminhos anteriores
        for (int i = 1; args[i] != NULL; i++) {
            adicionar_caminho(args[i]);
        }
    }
    else if (strcmp(args[0], "cat") == 0)
    {
        int argc;
        exibir_conteudo_arquivos(i, args);
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
    }
    else
    {
        pid_t pid = fork();
        if (pid == 0) // Processo filho
        {
            for (int i = 0; i < num_caminhos; i++) {
                char caminho_programa[MAX_COMANDO];
                snprintf(caminho_programa, sizeof(caminho_programa), "%s/%s", caminhos_executaveis[i], args[0]);
                if (access(caminho_programa, X_OK) == 0) {
                    execvp(caminho_programa, args);
                    perror("Erro ao executar programa");
                    exit(1); // Indique que houve um erro
                }
            }
            perror("Comando não encontrado");
            exit(1);
        }
        else if (pid > 0) // Processo pai
        {
            int status;
            waitpid(pid, &status, 0); // Espera o processo filho terminar
        }
        else
        {
            perror("Erro ao criar processo");
        }
    }

    return 1;  // Indica para continuar no shell
}

int main()
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

        int continuar = verificar_comandos(comando, args);
        if (!continuar) {
            break; // Se o comando for "exit", sai do loop e termina o shell
        }

    }

    return 0;
}
