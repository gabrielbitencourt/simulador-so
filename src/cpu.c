#include <stdlib.h>
#include <stdio.h>
#include "process.h"
#include "cpu.h"

extern FILE *output;
extern int quiet;

const int ALPHA = 5;
const int DISK_LENGTH = 5;
const int TAPE_LENGTH = 9;
const int PRINTER_LENGTH = 20;

void start(CPU *cpu, int queue_size)
{
    cpu->tick = 0;
    cpu->queue = malloc(sizeof(Process *) * queue_size);
    cpu->queue_size = queue_size;
    cpu->processing = -1;
    cpu->free_space = 0;
    cpu->finished = 0;
    cpu->processes_number = 0;
}

CPUState tick(CPU *cpu)
{
    if (quiet == 0)
        fprintf(output, "\n\ntick %d\n", cpu->tick);
    if (cpu->processes_number == 0)
    {
        if (quiet == 0)
            fprintf(output, "Não há nenhum processo na fila, esperando próximo processo...\n");
        cpu->tick++;
        return WAITED;
    }

    if (cpu->processing == -1)
    {
        int next = get_next_process(cpu);
        if (next >= 0)
        {
            if (quiet == 0)
                fprintf(output, "Não há outro processo para execução, continuando processo anterior...\n");
            swap_to_process(cpu, next, 0);
            cpu->started_processing = cpu->tick;
        }
        else
        {
            if (quiet == 0)
                fprintf(output, "Não há nenhum processo na fila, esperando próximo processo...\n");
            cpu->tick++;
            return WAITED;
        }
    }

    if (cpu->processing >= 0)
    {
        // processa por 1 tick


        // verifica se processo finalizou
        if (cpu->queue[cpu->processing]->length == cpu->queue[cpu->processing]->processed)
        {
            if (quiet == 0)
                fprintf(output, "Processo %s finalizado\n", cpu->queue[cpu->processing]->name);
            cpu->queue[cpu->processing] = NULL;
            cpu->finished = cpu->finished + 1;

            if (cpu->finished == cpu->processes_number)
            {
                if (quiet == 0)
                    fprintf(output, "Não há mais processos na fila.\n");
                cpu->tick++;
                return PROCESSED;
            }

            // verifica se tem algum processo na fila aguardando alem do atual, se não coloca a cpu em espera
            int next = get_next_process(cpu);
            if (next >= 0)
                swap_to_process(cpu, next, 1);
            else
                start_waiting(cpu);
        }
        // verifica se ja processou por mais do tempo limite por processo
        else if (cpu->tick - cpu->started_processing >= ALPHA - 1)
        {
            if (quiet == 0)
                fprintf(output, "Tempo limite de execução do processo %s\n", cpu->queue[cpu->processing]->name);

            // verifica se tem algum processo na fila aguardando alem do atual, se não coloca a cpu em espera
            int next = get_next_process(cpu);
            if (next >= 0)
                swap_to_process(cpu, next, 1);
            else
                start_waiting(cpu);
        }
    }
    cpu->tick++;
    return PROCESSED;
}

int new_process(CPU *cpu, Process *p)
{
    if (cpu->queue_size - cpu->processes_number + cpu->finished <= 0)
    {
        cpu->queue_size = cpu->queue_size * 2;
        cpu->queue = realloc(cpu->queue, sizeof(Process *) * cpu->queue_size);
    }

    if (quiet == 0)
        fprintf(output, "Adicionando processo %s a fila de espera no tick %d\n", p->name, cpu->tick);
    cpu->queue[cpu->free_space] = p;
    if (cpu->processing == -1)
    {
        if (quiet == 0)
            fprintf(output, "Fila de espera estava vazia, o processo será executado agora\n\n");
        cpu->processing = cpu->free_space;
        cpu->started_processing = cpu->tick;
    }
    cpu->processes_number = cpu->processes_number + 1;
    cpu->free_space = cpu->free_space + 1;
    return 0;
}

int get_next_process(CPU *cpu)
{
    int next = -1;
    for (int l = (cpu->processing + 1) % cpu->processes_number; l != cpu->processing; l = (l + 1) % cpu->processes_number)
    {
        if (cpu->queue[l] != NULL)
        {
            next = l;
            break;
        }
    }
    return next;
}

void swap_to_process(CPU *cpu, int pid, int changed)
{
    if (quiet == 0 && changed == 1)
        fprintf(output, "Trocando para processo %s\n", cpu->queue[pid]->name);
    cpu->processing = pid;
    cpu->started_processing = cpu->tick + 1;
}

void start_waiting(CPU *cpu)
{
    if (quiet == 0)
        fprintf(output, "Não há outro processo na fila\n");
    cpu->processing = -1;
}

void process(CPU *cpu, int pid)
{
    Process *current = cpu->queue[cpu->processing];
    current->processed = current->processed + 1;
    if (quiet == 0)
        fprintf(output, "Executando processo %s, faltam %d ticks para término\n", current->name, current->length - current->processed);
}