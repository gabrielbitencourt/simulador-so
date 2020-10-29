# Ambiente de Compiladores

Ambiente de compilação para a disciplina Compiladores do Departamento de Ciência da Computação da UFRJ, no Período Letivo Excepcional de 2020.

## Pré-requisitos

- Docker
- Make
- Readlink

Nos sistemas baseados em Linux, basta usar o gerenciador de pacotes (apt, dnf, pacman, etc.) para instalar o Docker.

No Windows, a maneira mais simples é instalar o [WSL2](https://docs.microsoft.com/pt-br/windows/wsl/install-win10) e o [Docker Desktop for Windows](https://docs.docker.com/docker-for-windows/install/). Depois disso, ative o back-end do WSL2 nas configurações do Docker Desktop.

## Execução

1. Abra um terminal
2. Clone o projeto (`git clone https://github.com/DCarts/Ambiente-Compiladores-PLE-UFRJ.git`)
3. Entre na pasta do projeto (`cd Ambiente-Compiladores-PLE-UFRJ`)
4. Construa a imagem do container de compiladores (`make imagem`). Pode ser necessário usar sudo.

Agora, basta executar `./run.sh <comando>` que o comando será executado num ambiente com gcc 7.3.0, flex 2.6.4 e bison 3.0.4, idênticos ao usado pelo professor para avaliação dos trabalhos.

Pode-se executar também `make shell` para obter uma shell iterativa no container com esses programas instalados.

O container só consegue enxergar o conteúdo da pasta codigo_fonte, então o código fonte de seu compilador deve estar lá.

Neste projeto também há um exemplo de Makefile que poderá ser usada para facilitar o desenvolvimento dos trabalhos de Compiladores.
