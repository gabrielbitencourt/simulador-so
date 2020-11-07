#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "map.h"
#include "queue.h"
#include "io.h"
#include "process.h"
#include "cpu.h"

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

extern FILE *output;
extern int quiet;

const int ALPHA = 5;

void start(CPU *cpu, int queue_size)
{
    cpu->tick = 0;
    cpu->started_processing = -1;
    cpu->hp_queue = *init_queue(20, PROCESS_Q);
    cpu->lp_queue = *init_queue(20, PROCESS_Q);
    cpu->disk_queue = *init_queue(20, IO_Q);
    cpu->tape_queue = *init_queue(20, IO_Q);
    cpu->printer_queue = *init_queue(20, IO_Q);
    cpu->executing = NULL;
}

CPUState tick(CPU *cpu)
{
    // verifica se processo finalizou ou esgotou tempo limite
    if (cpu->executing != NULL && (cpu->executing->length == cpu->executing->processed || cpu->tick - 1 - cpu->started_processing >= ALPHA - 1))
    {
        Queue *current_queue;
        if (current(&cpu->hp_queue) == cpu->executing)
            current_queue = &cpu->hp_queue;
        else
            current_queue = &cpu->lp_queue;

        cpu->executing = NULL;
        Process *ended = pop(current_queue);
        if (ended->length > ended->processed)
        {
            push(&cpu->lp_queue, ended);
        }

        if (cpu->hp_queue.length)
        {
            cpu->executing = current(&cpu->hp_queue);
            cpu->started_processing = cpu->tick;
            if (quiet == 0)
            {
                if (ended->length > ended->processed)
                    fprintf(output, "Tempo limite de execução do processo %s, movendo para fila de baixa prioridade e trocando para processo %s de alta prioridade.\n", ended->name, cpu->executing->name);
                else
                    fprintf(output, "Processo %s finalizado, trocando para processo %s de alta prioridade.\n", ended->name, cpu->executing->name);
            }
        }
        else if (cpu->lp_queue.length)
        {
            cpu->executing = current(&cpu->lp_queue);
            cpu->started_processing = cpu->tick;
            if (quiet == 0)
            {
                if (ended->length > ended->processed)
                    fprintf(output, "Tempo limite de execução do processo %s, movendo para fila de baixa prioridade e trocando para processo %s de baixa prioridade.\n", ended->name, cpu->executing->name);
                else
                    fprintf(output, "Processo %s finalizado, trocando para processo %s de baixa prioridade.\n", ended->name, cpu->executing->name);
            }
        }
        else
        {
            if (quiet == 0)
            {
                if (ended->length > ended->processed)
                    fprintf(output, "Tempo limite de execução do processo %s.\n", ended->name);
                else
                    fprintf(output, "Processo %s finalizado", ended->name); 
            }
        }
    }

    if (quiet == 0)
        fprintf(output, "\n\n" COLOR(WHITE_BG, BLACK_FG, "TICK %d") "\n", cpu->tick);

    if (cpu->hp_queue.length == 0 && cpu->lp_queue.length == 0 && cpu->disk_queue.length == 0 && cpu->tape_queue.length == 0 && cpu->printer_queue.length == 0)
    {
        if (quiet == 0)
            fprintf(output, "Não há processos em nenhuma fila, aguardando...\n");
        cpu->tick++;
        return WAITED;
    }
    else if (cpu->executing == NULL || (cpu->hp_queue.length > 0 && cpu->executing != current(&cpu->hp_queue)))
    {
        if (cpu->hp_queue.length)
        {
            if (quiet == 0)
            {
                Process *next = current(&cpu->hp_queue);
                if (cpu->executing == NULL)
                    fprintf(output, "Trocando para processo %s de alta prioridade.\n", next->name);
                else
                    fprintf(output, "Processo %s de alta prioridade entrou na fila, substituindo o processo %s de baixa prioridade.\n", next->name, cpu->executing->name);
            }
            cpu->executing = current(&cpu->hp_queue);
            cpu->started_processing = cpu->tick;
        }
        else if (cpu->lp_queue.length)
        {
            cpu->executing = current(&cpu->lp_queue);
            cpu->started_processing = cpu->tick;
            if (quiet == 0)
                fprintf(output, "Trocando para processo %s de baixa prioridade.\n", cpu->executing->name);
        }
        else 
        {
            if (quiet == 0)
                fprintf(output, "Filas de processos estão vazias.\n");
        }
    }

    // processa por 1 tick
    if (cpu->executing != NULL && !requests_io(cpu, cpu->executing))
        execute(cpu, cpu->executing);

    execute_ios(cpu);
    cpu->tick++;
    return PROCESSED;
}

void new_process(CPU *cpu, Process *p)
{
    push(&cpu->hp_queue, p);
    if (quiet == 0)
        fprintf(output, "Adicionando processo %s a fila de alta prioridade no tick %d\n", p->name, cpu->tick);
}

void execute(CPU *cpu, Process *p)
{
    p->processed = p->processed + 1;
    if (quiet == 0)
    {
        if (p->length - p->processed > 0)
            fprintf(output, "Executando processo %s, faltam %d ticks para finalizar\n", p->name, p->length - p->processed);
        else
            fprintf(output, "Processo %s finalizado.\n", p->name);
    }
}

void execute_ios(CPU *cpu)
{
    Queue *queues[3] = { &cpu->disk_queue, &cpu->tape_queue, &cpu->printer_queue };
    Queue *to_queue[3] = { &cpu->lp_queue, &cpu->hp_queue, &cpu->hp_queue };

    for (int i = 0; i < 3; i++)
    {
        Queue *q = queues[i];
        IO *io = current(q);
        if (io != NULL && cpu->tick > io->requested_at)
        {
            io->processed = io->processed + 1;
            if (quiet == 0 && io->length - io->processed > 0)
                fprintf(output, "Executando I/O de %s do processo %s, faltam %d ticks para finalizar e voltar a fila de %s.\n", name(io->type), io->process->name, io->length - io->processed, i == 0 ? "baixa prioridade" : "alta prioridade");

            if (io->processed == io->length)
            {
                pop(q);
                Process *p = io->process;
                if (p->processed < p->length)
                {
                    push(to_queue[(int) io->type], p);
                    if (quiet == 0)
                        fprintf(output, "I/O de %s do processo %s finalizou, movendo para fila de %s.\n", name(io->type), io->process->name, io->type == DISK ? "baixa prioridade" : "alta prioridade");
                }
                else
                {
                    if (quiet == 0)
                        fprintf(output, "I/O de %s do processo %s finalizou, e o processo já havia sido totalmente processado.\n", name(io->type), io->process->name);
                }
            }
        }
        else
        {
            if (quiet == 0)
                fprintf(output, "Fila de I/O de %s está vazia.\n", name((IOType)i));
        }
    }
}

bool requests_io(CPU *cpu, Process *p)
{
    int n_ios = count(p->ios, p->processed + 1);
    if (n_ios > 0)
    {
        IO **ios = (IO **)get(p->ios, p->processed + 1);
        for (int i = 0; i < n_ios; i++)
        {
            ios[i]->requested_at = cpu->tick;
            IOType type = ios[i]->type;
            if (quiet == 0)
                fprintf(output, "Executando processo %s foi requisitado I/O de %s\n", p->name, name(type));

            // remove processo da fila
            if (current(&cpu->hp_queue) == p)
            {
                pop(&cpu->hp_queue);
                cpu->executing = NULL;
            }
            else if (current(&cpu->lp_queue) == p)
            {
                pop(&cpu->lp_queue);
                cpu->executing = NULL;
            }

            switch (type)
            {
                case DISK:
                    push(&cpu->disk_queue, ios[i]);
                    break;

                case TAPE:
                    push(&cpu->tape_queue, ios[i]);
                    break;

                case PRINTER:
                    push(&cpu->printer_queue, ios[i]);
                    break;
            }
        }
        p->processed = p->processed + 1;
        return true;
    }
    return false;
}