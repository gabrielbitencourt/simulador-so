#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "process.h"
#include "cpu.h"

#define OPTSTR "qs:i:o:"

typedef struct Args
{
    int speed;
    int quiet;
    FILE *input;
    FILE *output;
} Args;

FILE* output;
int quiet;

int main(int argc, char *argv[])
{
    /* Leitura de argumentos */
    int arg;
    Args args = { 1, 0, stdin, stdout };
    output = stdout;
    quiet = 0;

    while ((arg = getopt(argc, argv, OPTSTR)) != EOF)
    {
        switch (arg)
        {
            case 'q':
                args.quiet = 1;
                quiet = 1;
                break;

            case 's':
                args.speed = atoi(optarg);
                break;

            case 'i':
                args.input = fopen(optarg, "r");
                if (!args.input)
                {
                    fprintf(args.output, "Não foi possível abrir o arquivo de input. Lendo de STDIN.");
                    args.input = stdin;
                }
                break;

            case 'o':
                args.output = fopen(optarg, "w");
                if (!args.output)
                {
                    args.output = stdout;
                    fprintf(args.output, "Não foi possível abrir o arquivo de output. Escrevendo em STDOUT.");
                }
                output = args.output;
                break;

            default:
                printf("def arg\n");
                break;
        }
    }
    
    /* Setup do simulador (construção da tabela de processos) */
    int n = 0, max_time = 200;
    StartTimeMap map;

    if (args.input == stdin && args.output == stdout) 
        fprintf(args.output, "Número de processos a serem escalonados: ");
    fscanf(args.input, "%d", &n);

    map = init(max_time);

    char str[100];
    for (int i = 0; i < n; i++)
    {
        int start, length, m;
        size_t str_size;
        char *p_name;
        IO *ios;
        Process p;

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "\n\nNome do processo %d: ", i + 1);
        fscanf(args.input, "%s", str);

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "Tempo de início do processo %s (CPU começa em t=0): ", str);
        fscanf(args.input, "%d", &start);

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "Tempo de serviço do processo %s: ", str);
        fscanf(args.input, "%d", &length);

        if (f == stdin)
            fprintf(args.output, "Número de I/Os do processo %s: ", str);
        fscanf(args.input, "%d", &m);

        ios = malloc(sizeof(IO) * m);
        for (int j = 0; j < m; j++)
        {
            int type, time;
            if (args.input == stdin && args.output == stdout)
                fprintf(args.output, "\n\tTipo da %dª I/O (1: Disco, 2: Fita Magnética, 3: Impressora): ", j + 1);
            fscanf(args.input, "%d", &type);

            if (args.input == stdin && args.output == stdout)
                fprintf(args.output, "\tMomento da %dª operação de IO (em relação ao tempo de serviço do processo): ", j + 1);
            fscanf(args.input, "%d", &time);

            IO io;
            io.start = time;
            io.type = (IOType)type;
            ios[j] = io;
        }

        str_size = strlen(str);
        p_name = malloc(str_size);
        strcpy(p_name, str);

        p.name = p_name;
        p.start = start;
        p.length = length;
        p.io = ios;
        p.processed = 0;
        insert(&map, start, p);
    }

    if (args.speed > 0) sleep(args.speed);

    /* Início da simulação */
    CPU cpu;
    start(&cpu, n);
    while (cpu.finished < n)
    {
        int starting_now = count(&map, cpu.tick);
        if (starting_now > 0)
        {
            Process *starting_processes = get(&map, cpu.tick);
            if (args.quiet == 0) fprintf(args.output, "\n\n");
            for (int k = 0; k < starting_now; k++)
            {
                new_process(&cpu, &starting_processes[k]);
            }
        }
        tick(&cpu);
        if (args.speed > 0) sleep(args.speed);
    }

    fprintf(args.output, "Todos os processos foram finalizados... press F to pay your respect\n");
    return 0;
}
