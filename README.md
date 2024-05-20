# Shell

Este projeto implementa um shell simples em C, capaz de executar comandos básicos do sistema operacional. O shell suporta comandos internos como exit, cd, path, cat, e ls, além de permitir a execução de programas externos e comandos paralelos.

## Comandos 
- Finaliza o Shell.
```sh
exit
```
- Muda o diretório de trabalho.
```sh
cd <diretorio>/<diretorio2>
```
- Define caminho(s) para busca de executáveis.
Considera o diretorio atual, portanto deve-se definir o caminho no path a partir do diretorio atual
```sh
path <diretorio>/<diretorio2>
```
- Execução de programas externo. 
O shell deverá procurar, em todos os caminhos definidos com o built-in path, por um executável com o nome inserido no comando
```sh
<arquivo>
```
- O binário cat <arquivo> lê o conteúdo do arquivo no argumento e o escreve na saída padrão.
```sh
cat <arquivo>
```
- O binário ls lista o conteúdo do diretório atual. O ls suporta os parâmetros -l, -a, -la e -al conforme o funcionamento do ls original.
```sh
ls 
```
- A saída dos comandos poderá ser redirecionada para um arquivo.
```sh
cat arquivo.txt > arquivo-saída
```
### Vários comandos separados por um & são executados de forma concorrente, em processos separados.
```sh
cmd1 & cmd2 arg1 arg2 & cmd3 arg3
```
