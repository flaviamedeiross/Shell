# Shell

### Descrição do Projeto
- Este projeto implementa um shell simples em C, capaz de executar comandos básicos do sistema operacional. O shell suporta comandos internos como exit, cd, path, cat, e ls, além de permitir a execução de programas externos e comandos em paralelo.

### Status do Projeto
- Concluido

### Execução e Compilação do Programa

### Para axecução do Shell Iterativo siga os comandos abaixo:
```sh
gcc -g -o cat cat.c
```
```sh
gcc -g -o ls ls.c
```
```sh
gcc -g -o shell shell.c
```
```sh
./shell
```
- Primeiro comando necessário dentro do Shell
```sh
path /home/<seu_usuario>/<caminho_da_pasta>
```
> Caso não seja passado o caminho path correto (onde se encontra os seus arquivos desse repositório), os comandos cat e ls não irão funcionar

### Para a Utilização do Shell Batch siga os comando abaixo:
- Repita os comando a seguir caso não tenha executado o Shell Iterativo antes: `gcc -g -o cat cat.c` , `gcc -g -o ls ls.c` 
```sh
gcc -g -o shell shell.c
```
```sh
./shell comandos.txt
```
> Caso de erro nos comandos do arquivo Comandos.txt execute os comandos abaixo:
```sh
sudo apt-get install dos2unix
```
```sh
dos2unix comandos.txt
```

### Funcionalidades
- Finaliza o Shell.
```sh
exit
```
- Muda o diretório de trabalho.
```sh
cd <diretorio>/<diretorio2>
```
- Define caminho(s) para busca de executáveis.
  
O caminho todo deverá ser colocado para a execução desejada.
```sh
path <diretorio>/<diretorio2>
```

- O binário cat <arquivo> lê o conteúdo do arquivo no argumento e o escreve na saída padrão.
```sh
cat <arquivo>
```
- A saída dos comandos poderá ser redirecionada para um arquivo. Um exemplo de comando pode ser visto abaixo:
```sh
cat arquivo.txt > arquivo-saída
```
- O binário ls lista o conteúdo do diretório atual.

O ls suporta os parâmetros -l, -a, -la e -al conforme o funcionamento do ls original.
```sh
ls 
```
- Vários comandos separados por um & são executados de forma concorrente, em processos separados.
```sh
cmd1 arg1 & cmd2 arg2 & cmd3 arg3
```


### Tecnologias Utilizadas
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white) ![VSCode](https://img.shields.io/badge/VSCode-0078D4?style=for-the-badge&logo=visual%20studio%20code&logoColor=white) ![virtualbox](https://img.shields.io/badge/VirtualBox-183A61?logo=virtualbox&logoColor=white&style=for-the-badge)

### Pessoas Desenvolvedoras: 
- Daniela Akemi Hayashi
- Flávia Cristina Medeiros
- Giovana Salazar Alarcon
