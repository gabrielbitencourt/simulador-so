#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include "map.h"
#include "queue.h"
#include "io.h"
#include "process.h"
#include "cpu.h"

#define OPTSTR "qs:i:o:"

#define COLOR(background, foreground, string) "\x1b[" foreground ";" background "m" string "" \
                                              "\x1b[0;0m"

#define DEF "0"
#define RED_BG "41"
#define RED_FG "31"
#define GREEN_BG "42"
#define GREEN_FG "32"
#define YELLOW_BG "43"
#define YELLOW_FG "33"
#define BLUE_BG "44"
#define BLUE_FG "34"
#define PINK_BG "45"
#define PINK_FG "35"
#define CYAN_BG "46"
#define CYAN_FG "36"
#define BLACK_FG "30"
#define WHITE_BG "47"

typedef struct Args
{
    int speed;
    int quiet;
    FILE *input;
    FILE *output;
} Args;

FILE *output;
int quiet;

int main(int argc, char *argv[])
{
    /* Leitura de argumentos */
    int arg;
    Args args = {1, 0, stdin, stdout};
    output = stdout;
    quiet = 0;

    Map *map;

    CPU cpu;

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

    /* Setup do simulador (construção da tabela de processos e tabela de I/O de cada processo) */
    int n = 0, max_time = 200;
    map = init(max_time, PROCESS_M);

    if (args.input == stdin && args.output == stdout)
        fprintf(args.output, "Número de processos a serem escalonados: ");
    fscanf(args.input, "%d", &n);

    char str[100];
    for (int i = 0; i < n; i++)
    {
        Process *p;
        int start, length, m;
        size_t str_size;
        char *p_name;
        Map *ios;

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "\n\nNome do processo %d: ", i + 1);
        fscanf(args.input, "%s", str);

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "Tempo de início do processo %s (CPU começa em t=0): ", str);
        fscanf(args.input, "%d", &start);

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "Tempo de serviço do processo %s: ", str);
        fscanf(args.input, "%d", &length);

        if (args.input == stdin && args.output == stdout)
            fprintf(args.output, "Número de I/Os do processo %s: ", str);
        fscanf(args.input, "%d", &m);

        str_size = strlen(str);
        p_name = malloc(str_size);
        strcpy(p_name, str);

        p = create(p_name, start, length, m);

        ios = init(length, IO_M);
        if (quiet == 0)
            fprintf(output, " %s;\n", str);

        for (int j = 0; j < m; j++)
        {
            int type, time;
            if (args.input == stdin && args.output == stdout)
                fprintf(args.output, "\n\tMomento da %dª operação de IO (em relação ao tempo de serviço do processo na CPU): ", j + 1);
            fscanf(args.input, "%d", &time);

            if (args.input == stdin && args.output == stdout)
                fprintf(args.output, "\tTipo da %dª I/O (1: Disco, 2: Fita Magnética, 3: Impressora): ", j + 1);
            fscanf(args.input, "%d", &type);

            IO *io = create_io((IOType) (type - 1), p, time);
            insert(ios, time, (void *) io);
            if (quiet == 0)
                fprintf(output, "\n");
        }

        p->ios = ios;
        insert(map, start, (void *)p);
    }

    if (args.speed > 0)
        sleep(args.speed);

    /* Início da simulação */
    int started = 0;
    start(&cpu, n);

    // se todos os processos já foram iniciados e todas as filas estão vazias, podemos encerrar CPU
    while (started < n || cpu.hp_queue.length > 0 || cpu.lp_queue.length > 0 || cpu.disk_queue.length > 0 || cpu.tape_queue.length > 0 || cpu.printer_queue.length > 0)
    {
        int starting_now = count(map, cpu.tick);
        if (starting_now > 0)
        {
            void **starting_processes = get(map, cpu.tick);
            if (args.quiet == 0)
                fprintf(args.output, "\n\n");
            for (int k = 0; k < starting_now; k++)
            {
                new_process(&cpu, (Process *)starting_processes[k]);
            }
            started = started + starting_now;
        }
        tick(&cpu);
        if (args.speed > 0)
            sleep(args.speed);
    }

    fprintf(args.output, "\nTodos os processos foram finalizados..." COLOR(BLUE_BG, DEF, "press %c to pay your respect") "...\n", 'F');
    return 0;
}
