# Trabalho final - Banco de Dados PLE
## Simulador de escalonador de processos

Esse README tem como finalidade explicar e exemplificar alguns dos pontos mais práticos do simulador. Para a descrição completa acesse o relatório do trabalho [aqui](https://docs.google.com/document/d/1gABO4QzEpfpvpKrXuyFM65EGWOOAtwwMjshfHCgcttM/edit?usp=sharing).

### Requisitos para rodar o projeto

- C (ambiente e compilador)

### Instruções para compilação do projeto em sistema Unix
1. Entre no diretório `src` e execute o seguinte comando: `make`

### Instruções para compilação do projeto em sistema Windows via docker (não testado)
1. Entre no diretório raiz e execute o seguinte comando: `make`
2. Será aberto um shell de um container Unix dentro da pasta `src`
3. Execute o comando `make`


### Instruções para execução do simulador

O simulador conta com alguns argumentos de execução para facilitar a simulação de diferentes cenários. Todos eles são opcionais e podem ser usados em qualquer ordem. São eles:

- `-a int`: Quantum. Recebe o valor para o quantum de troca de processos. Caso não seja fornecido o valor padrão é 5. Formato: `./simulador -a 3`
- `-s int`: Velocidade real de uma unidade de tempo. É o tempo que o programa ficará em espera entre a simulação das unidades de tempo da CPU. O valor padrão é 1, ou seja, entre cada unidade de tempo da CPU o programa espera 1 segundo. Para uma execução mais rápida é possível usar o valor 0, que não há espera real entre as unidades de tempo da simulação. Formato: `./simulador -s 0`.
- `-i file`: Arquivo de input. Recebe caminho para arquivo de texto com as entradas para o programa. Caso não seja fornecido as entradas são lidas de `STDIN`. Formato: `./simulador -i input.txt`
- `-o file`: Arquivo de output. Recebe caminho para arquivo de texto (caso o arquivo não exista será criado) onde será escrita a saída do programa. Caso não seja fornecido a saída é escrita em `STDOUT`. Caso seja fornecido um output válido, por padrão o tempo de velocidade real da unidade de tempo é setado como zero, ou seja, é fornecido implicitamente a opção `-s 0`. Formato: `./simulador -o output.txt`
- `-r file`: Arquivo para salvar entradas digitadas em `STDIN`. Quando esse argumento é utilizado toda entrada lida de `STDIN` é escrita no arquivo para que possa ser reutilizada mais vezes sem ter que ficar digitando em toda execução. Não pode ser usado junto a opção de input `-i`. Formato: `./simulador -r input.txt`
- `-n`: Remove cores da saída. Utilizar caso o terminal em uso não tenha suporte aos ANSI escape codes. Para aumentar a compatibilidade com diferentes terminais de comando, quando essa opção é fornecida, não são adicionados modificadores de cor na saída do programa. Esse argumento é passado implicitamente com a opção `-o`, ou seja, quando a saída não é `STDOUT` não são impressos os códigos ANSI de cores.

### Formato de entrada esperado pelo simulador

É recomendado utilizar o simulador pela primeira vez sem nenhum argumento para que se entenda melhor a entrada e a saída do programa. Quando o prgrama lê de `STDIN` são impressas na tela instruções do que deve ser fornecido em cada momento para montagem da tabela de processos e da tabela de I/O para cada processo.

A entrada é lida na seguinte ordem:

Número de processos a serem escalonados (x >= 0)
:Para cada processo n
    Nome do processo n
    Tempo de início do processo n (0 <= x <= 200)
    Tempo de serviço do processo n (x >= 0)
    Número de I/Os do processo n (x >= 0)
    :Para cada operação de I/O m do processo n
        Momento da m-gésima operação de IO, em relação ao tempo de serviço do processo na CPU (1 <= x < tempo total de serviço do processo)
        Tipo da m-gésima I/O (1: Disco, 2: Fita Magnética, 3: Impressora)

### Formato de saída do simulador

Antes de iniciar a simulação o programa escreve a tabela de Processos+IO recebida da entrada escolhida. Por exemplo:

Nome do processo/IO   Tempo de inicio     Tempo de servico    
A                     0                   7                   
Disco                 3                   5
Fita                  5                   9

B                     2                   8                   

C                     0                   4                   
Impressora            3                   20

Nesse exemplo, serão executados 3 processos A, B e C com duração de 7, 8 e 4, respectivamente. Sendo que o processo A realiza 2 operações de I/O, uma de disco na 3ª unidade de tempo e uma de fita na 5ª unidade de tempo. O processo B não realiza I/O e o processo C realiza uma operação de I/O de impressora na sua 3ª unidade de tempo. Note que o tempo de serviço das operações de I/O são definidos nas premissas do trabalho e são iguais para todos os processos.

Após o início da simulação, podemos dividir a execução do simulador em 2 partes: introdução de novos processos e processamento.

Introdução de novos processos:
A cada início de ciclo o simulador verifica na tabela de processos se há algum processos que entrará naquele ciclo. Caso haja, ele é inserido.

Processamento:
A cada ciclo de processamento o processador simula algumas ações, são elas troca de contexto (caso necessário), execução do processo e execução de I/O.

Note que a troca de contexto não conta como uma unidade de tempo de processamento, pois ela ocorre sempre antes da próxima execução. As condições para troca de contexto são::
1. Se o processo em execução atual esgotou o tempo limite e há outro processo em qualquer fila;
2. Se o processo em execução atual finalizou o processamento e há outro processo em qualquer fila;
3. Se o processo em execução atualmente é da fila de baixa prioridade e há um  processo na fila de alta prioridade;
4. Se não há processo em execução atualmente e há outro processo em qualquer fila.


A execução de processo e execução de I/O são bem similares, ambas funcionam em filas (detalhadas a seguir) e a cada unidade de tempo é simulado o comportamento de um processo real que executa um número x de instruções. Quando um processo requisita uma operação de I/O no tempo t ele é alocado na fila da I/O e começará a processar essa operação a partir de t + 1 (caso a fila dessa I/O esteja livre). Esse tempo t conta como uma unidade de tempo alocada para a execução daquele processo.

Filas
As filas são usadas para execução de processos e operações de I/O. As filas de processo são a fila de alta prioridade e a fila de baixa prioridade. As filas de I/O são as filas de Disco, de Fita e de Impressora.

A diferenção entre a execução de processos e I/Os é que as filas de processo são executas uma por vez, ou seja, ou é executado um processo da fila de alta prioridade ou é executado um processo da fila de baixa prioridade.

Já as fila de I/O são todas executadas uma vez em uma unidade de tempo.

Caso alguma fila esteja vazia nada é executado nela.

Considerando todos esses pontos, a cada unidade de tempo a saída do programa é:

TICK N
[Troca de Contexto]: Motivo e descrição da troca de contexto
[Fila de Processos]: Qual processo foi executado, de qual fila ele era e quanto tempo de processamento falta para ele finalizar.
[Fila I/O de Disco]: Qual processo teve sua I/O executada e quanto tempo falta para a operação finalizar
[Fila I/O de Fita ]: Qual processo teve sua I/O executada e quanto tempo falta para a operação finalizar
[Fila I/O de Impr.]: Qual processo teve sua I/O executada e quanto tempo falta para a operação finalizar

Além disso antes de cada unidade de tempo é também escrito na saída qual o estado atual das filas que não estão vazias, para facilitar o entendimento do que será processado na próxima unidade de tempo.
