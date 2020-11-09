#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include "helper.h"
#include "map.h"
#include "queue.h"
#include "io.h"
#include "process.h"
#include "cpu.h"

#define OPTSTR "ns:i:o:r:a:"

typedef struct Args
{
    int alpha;
    int speed;
    bool colored;
    FILE *input;
    FILE *output;
    FILE *record;
} Args;

FILE *output;
bool colored = true;
int ALPHA = 5;

int main(int argc, char *argv[])
{
    /* Leitura de argumentos */
    int arg;
    Args args = {5, 1, true, stdin, stdout, NULL};
    output = stdout;

    Map *map;

    CPU cpu;

    while ((arg = getopt(argc, argv, OPTSTR)) != EOF)
    {
        switch (arg)
        {
        case 's':
            if (output == stdout)
                args.speed = atoi(optarg);
            break;

        case 'i':
            args.input = fopen(optarg, "r");
            if (!args.input)
            {
                fprintf(args.output, "Não foi possível abrir o arquivo de input. Lendo de STDIN.\n");
                args.input = stdin;
            }
            else
                args.record = NULL;
            break;

        case 'a':
            args.alpha = atoi(optarg);
            if (args.alpha > 0)
                ALPHA = args.alpha;
            break;

        case 'o':
            args.output = fopen(optarg, "w");
            if (!args.output)
            {
                args.output = stdout;
                fprintf(args.output, "Não foi possível abrir o arquivo de output. Escrevendo em STDOUT.\n");
            }
            output = args.output;
            if (output != stdout)
                args.speed = 0;
            break;

        case 'r':
            if (args.input == stdin)
            {
                args.record = fopen(optarg, "w");
                if (!args.record)
                {
                    args.record = fopen("record", "w");
                    fprintf(args.output, "Não foi possível abrir o arquivo `%s` de record. Escrevendo no arquivo `record`.\n", optarg);
                }
                else
                    fprintf(args.output, "Gravando input no arquivo `%s`.\n", optarg);
            }
            break;

        case 'n':
            args.colored = false;
            colored = false;
            break;

        default:
            printf("def arg\n");
            break;
        }
    }

    /* Setup do simulador (construção da tabela de processos e tabela de I/O de cada processo) */
    int n = 0, max_time = 200;
    map = init(max_time, PROCESS_M);

    if (args.input == stdin)
        printf("Número de processos a serem escalonados: ");
    fscanf(args.input, "%d", &n);
    if (args.record != NULL)
        fprintf(args.record, "%d\n", n);

    char str[100];
    for (int i = 0; i < n; i++)
    {
        Process *p;
        int start, length, m;
        size_t str_size;
        char *p_name;
        Map *ios;

        if (args.input == stdin)
            printf("\n\nNome do processo %d: ", i + 1);
        fscanf(args.input, "%s", str);
        if (args.record != NULL)
            fprintf(args.record, "%s\n", str);

        if (args.input == stdin)
            printf("Tempo de início do processo %s (CPU começa em t=0): ", str);
        fscanf(args.input, "%d", &start);
        if (args.record != NULL)
            fprintf(args.record, "%d\n", start);

        if (args.input == stdin)
            printf("Tempo de serviço do processo %s: ", str);
        fscanf(args.input, "%d", &length);
        if (args.record != NULL)
            fprintf(args.record, "%d\n", length);

        if (args.input == stdin)
            printf("Número de I/Os do processo %s: ", str);
        fscanf(args.input, "%d", &m);
        if (args.record != NULL)
            fprintf(args.record, "%d\n", m);

        str_size = strlen(str);
        p_name = malloc(str_size);
        strcpy(p_name, str);

        p = create(p_name, start, length, m);

        ios = init(length, IO_M);

        for (int j = 0; j < m; j++)
        {
            int type, time;
            if (args.input == stdin)
                printf("\n\tMomento da %dª operação de IO (em relação ao tempo de serviço do processo na CPU): ", j + 1);
            fscanf(args.input, "%d", &time);
            if (args.record != NULL)
                fprintf(args.record, "%d\n", time);

            if (args.input == stdin)
                printf("\tTipo da %dª I/O (1: Disco, 2: Fita Magnética, 3: Impressora): ", j + 1);
            fscanf(args.input, "%d", &type);
            if (args.record != NULL)
                fprintf(args.record, "%d\n", type);

            IO *io = create_io((IOType) (type - 1), p, time);
            insert(ios, time, (void *) io);
        }

        p->ios = ios;
        insert(map, start, (void *)p);
    }

    fclose(args.record);
    nprintf("%-22s%-20s%-20s\n", "Nome do processo/IO", "Tempo de inicio", "Tempo de servico");
    for (int i = 0; i < max_time; i++)
    {
        int starting_now = count(map, i);
        if (starting_now > 0)
        {
            void **starting_processes = get(map, i);
            for (int k = 0; k < starting_now; k++)
            {
                Process *p = starting_processes[k];
                nprintf("%-22s%-20d%-20d\n", p->name, p->start, p->length);
                for (int l = 1; l <= p->length; l++)
                {
                    int n_ios = count(p->ios, l);
                    if (n_ios > 0)
                    {
                        IO **ios = (IO **)get(p->ios, l);
                        for (int j = 0; j < n_ios; j++)
                        {
                            nprintf("%-22s%-20d%-20d\n", name(ios[j]->type), ios[j]->start, ios[j]->length);
                        }
                    }
                }
                nprintf("\n");
            }
        }
    }

    if (args.speed > 0)
        sleep(args.speed);

    /* Início da simulação */
    cprintf(GREEN_BG, DEF_FG, "Iniciando simulação da CPU...");
    int started = 0;
    start(&cpu, n);

    // se todos os processos já foram iniciados e todas as filas estão vazias, podemos encerrar CPU
    while (started < n || cpu.hp_queue.length > 0 || cpu.lp_queue.length > 0 || cpu.disk_queue.length > 0 || cpu.tape_queue.length > 0 || cpu.printer_queue.length > 0)
    {
        int starting_now = count(map, cpu.tick);
        if (starting_now > 0)
        {
            void **starting_processes = get(map, cpu.tick);
            nprintf("\n\n");

            for (int k = 0; k < starting_now; k++)
            {
                new_process(&cpu, starting_processes[k]);
            }
            started = started + starting_now;
        }

        nprintf("\n\n");
        if (cpu.hp_queue.length)
            nprintf("Estado da fila de alta prioridade : %s\n", stringify_queue(&cpu.hp_queue));
        if (cpu.lp_queue.length)
            nprintf("Estado da fila de baixa prioridade: %s\n", stringify_queue(&cpu.lp_queue));
        if (cpu.disk_queue.length)
            nprintf("Estado da fila de I/O Disco       : %s\n", stringify_queue(&cpu.disk_queue));
        if (cpu.tape_queue.length)
            nprintf("Estado da fila de I/O Fita        : %s\n", stringify_queue(&cpu.tape_queue));
        if (cpu.printer_queue.length)
            nprintf("Estado da fila de I/O Impressora  : %s\n", stringify_queue(&cpu.printer_queue));


        tick(&cpu);
        if (args.speed > 0)
            sleep(args.speed);
    }

    nprintf("\nTodos os processos foram finalizados...\n");    
    fclose(args.output);
    // free everything ;)
    return 0;
}
