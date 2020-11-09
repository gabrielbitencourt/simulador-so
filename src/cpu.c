#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "helper.h"
#include "map.h"
#include "queue.h"
#include "io.h"
#include "process.h"
#include "cpu.h"

extern int ALPHA;

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

void tick(CPU *cpu)
{
    nprintf("\n\n");
    cprintf(WHITE_BG, BLACK_FG, "TICK %d", cpu->tick);
    nprintf("\n");

    // verifica se processo atual finalizou ou esgotou tempo limite no tick anterior
    if (cpu->executing != NULL && (cpu->executing->length == cpu->executing->processed || cpu->tick - 1 - cpu->started_processing >= ALPHA - 1))
    {
        // procura qual a fila atual
        Queue *current_queue;
        if (current(&cpu->hp_queue) == cpu->executing)
            current_queue = &cpu->hp_queue;
        else
            current_queue = &cpu->lp_queue;

        // remove o processo de execucao e o coloca no fim da fila caso ainda necessite de tempo na CPU
        cpu->executing = NULL;
        Process *ended = pop(current_queue);
        if (ended->length > ended->processed)
        {
            push(&cpu->lp_queue, ended);
        }

        // verifica se há alguma processo na fila de alta prioridade, caso tenha ele deve ser executado
        // caso não tenha, verifica-se a fila de baixa prioridade
        // caso tambem esteja vazia, continua o processamento do atual
        // a não ser que ele também esteja finalizado, nesse caso a CPU fica aguardando
        cprintf(PINK_BG, DEF_FG, "[Troca de Contexto]:");
        nprintf(" ");

        if (cpu->hp_queue.length)
        {
            cpu->executing = current(&cpu->hp_queue);
            cpu->started_processing = cpu->tick;
            if (ended->length > ended->processed)
            {
                if (ended == cpu->executing)
                    nprintf("Tempo limite de execução do processo %s, mas não há outro processo na fila. Continunando sua execução.\n", ended->name);
                else
                    nprintf("Tempo limite de execução do processo %s, movendo para fila de baixa prioridade e trocando para processo %s de alta prioridade.\n", ended->name, cpu->executing->name);
            }
            else
                nprintf("Processo %s finalizado, trocando para processo %s de alta prioridade.\n", ended->name, cpu->executing->name);
        }
        else if (cpu->lp_queue.length)
        {
            cpu->executing = current(&cpu->lp_queue);
            cpu->started_processing = cpu->tick;
            if (ended->length > ended->processed)
            {
                if (ended == cpu->executing)
                    nprintf("Tempo limite de execução do processo %s, mas não há outro processo na fila. Continunando sua execução.\n", ended->name);
                else
                    nprintf("Tempo limite de execução do processo %s, movendo para fila de baixa prioridade e trocando para processo %s de baixa prioridade.\n", ended->name, cpu->executing->name);
            }
            else
                nprintf("Processo %s finalizado, trocando para processo %s de baixa prioridade.\n", ended->name, cpu->executing->name);
        }
        else
        {
            if (ended->length > ended->processed)
                nprintf("Tempo limite de execução do processo %s.\n", ended->name);
            else
                nprintf("Processo %s finalizado\n", ended->name);
        }
    }

    // inicio do processamento no tick
    // se todas as filas estiverem vazias, aguarda e retorna
    if (cpu->hp_queue.length == 0 && cpu->lp_queue.length == 0 && cpu->disk_queue.length == 0 && cpu->tape_queue.length == 0 && cpu->printer_queue.length == 0)
    {
        cprintf(GREEN_BG, BLACK_FG, "[Todas as filas%-3s]:", "");
        nprintf(" Não há processos em nenhuma fila, aguardando...\n");
        cpu->tick++;
        return;
    }
    // se não tiver executando ninguem sabendo que as filas não estão vazias, procura-se o próximo processo
    // ou caso a fila de alta prioridade tenha alguém que não é o processo atual, troca contexto para processo de alta prioridade
    else if (cpu->executing == NULL || (cpu->hp_queue.length > 0 && cpu->executing != current(&cpu->hp_queue)))
    {
        if (cpu->hp_queue.length)
        {
            Process *next = current(&cpu->hp_queue);
            if (cpu->executing == NULL)
            {
                cprintf(PINK_BG, DEF_FG, "[Troca de Contexto]:");
                nprintf(" Trocando para processo %s de alta prioridade.\n", next->name);
            }
            else
            {
                cprintf(PINK_BG, DEF_FG, "[Troca de Contexto]:");
                nprintf(" Processo %s de alta prioridade entrou na fila, substituindo o processo %s de baixa prioridade.\n", next->name, cpu->executing->name);
            }
            cpu->executing = current(&cpu->hp_queue);
            cpu->started_processing = cpu->tick;
        }
        else if (cpu->lp_queue.length)
        {
            cpu->executing = current(&cpu->lp_queue);
            cpu->started_processing = cpu->tick;

            cprintf(PINK_BG, DEF_FG, "[Troca de Contexto]:");
            nprintf(" Trocando para processo %s de baixa prioridade.\n", cpu->executing->name);
        }
        else
        {
            // filas de processo estão vazias, mas as de IO não
            cprintf(BLUE_BG, DEF_FG, "[Fila de Processos]:");
            nprintf(" Filas de processos estão vazias.\n");
        }
    }

    // processa por 1 tick, caso tenha processo e caso ele não necessite de IO nesse tick
    // caso ele necessite de IO, a função requests_io faz o processamento dele e troca o contexto após a interrupção
    if (cpu->executing != NULL && !requests_io(cpu, cpu->executing))
        execute(cpu, cpu->executing);

    // executa processos que estão na fila de IO
    execute_ios(cpu);
    cpu->tick++;
    return;
}

void new_process(CPU *cpu, Process *p)
{
    push(&cpu->hp_queue, p);
    nprintf("Adicionando processo %s a fila de alta prioridade no tick %d\n", p->name, cpu->tick);
}

void execute(CPU *cpu, Process *p)
{
    cprintf(BLUE_BG, DEF_FG, "[Fila de Processos]:");
    nprintf(" ");
    p->processed = p->processed + 1;
    if (p->length - p->processed > 0)
        nprintf("Executando processo %s, faltam %d ticks para finalizar\n", p->name, p->length - p->processed);
    else
        nprintf("Executando último tick do processo %s.\n", p->name);
}

void execute_ios(CPU *cpu)
{
    Queue *queues[3] = {&cpu->disk_queue, &cpu->tape_queue, &cpu->printer_queue};
    Queue *to_queue[3] = {&cpu->lp_queue, &cpu->hp_queue, &cpu->hp_queue};

    for (int i = 0; i < 3; i++)
    {
        Queue *q = queues[i];
        IO *io = current(q);
        // só começa a processar a IO se ela chegou no tick anterior ou antes
        // (para não executar IO no mesmo tick que executou o tick de interrupção)
        cprintf(YELLOW_BG, BLACK_FG, "[Fila I/O de %-5s]:", abrv(i));
        nprintf(" ");

        if (io != NULL && cpu->tick > io->requested_at)
        {
            io->processed = io->processed + 1;
            if (io->length - io->processed > 0)
                nprintf("Executando I/O de %s do processo %s, faltam %d ticks para finalizar e voltar a fila de %s.\n", name(io->type), io->process->name, io->length - io->processed, i == 0 ? "baixa prioridade" : "alta prioridade");

            if (io->processed == io->length)
            {
                pop(q);
                Process *p = io->process;
                if (p->processed < p->length)
                {
                    push(to_queue[(int)io->type], p);
                    nprintf("I/O de %s do processo %s finalizou, movendo para fila de %s.\n", name(io->type), io->process->name, io->type == DISK ? "baixa prioridade" : "alta prioridade");
                }
                else
                    nprintf("I/O de %s do processo %s finalizou, e o processo já havia sido totalmente processado.\n", name(io->type), io->process->name);
            }
        }
        else
            nprintf("Fila de I/O de %s está vazia.\n", name((IOType)i));
    }
}

bool requests_io(CPU *cpu, Process *p)
{
    int n_ios = count(p->ios, p->processed + 1);
    if (n_ios > 0)
    {
        cprintf(BLUE_BG, DEF_FG, "[Fila de Processos]:");
        nprintf(" ");
        IO **ios = (IO **)get(p->ios, p->processed + 1);
        for (int i = 0; i < n_ios; i++)
        {
            ios[i]->requested_at = cpu->tick;
            IOType type = ios[i]->type;
            nprintf("Executando processo %s foi requisitado I/O de %s\n", p->name, name(type));

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

            // coloca na fila adequada de IO
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