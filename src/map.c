#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helper.h"
#include "map.h"
#include "io.h"
#include "process.h"

extern FILE *output;

Map *init(int max_time, MapType type)
{
    Map *map = malloc(sizeof(Map));
    // if (type == PROCESS_M)
    //     nprintf("Inicializando tabela de processos com %d espaços de tempo;\n", max_time);
    // else
    //     nprintf("\nInicializando tabela de I/O do processo");

    int *count;
    count = malloc(sizeof(int) * max_time);

    void ***map_values;
    map_values = malloc(sizeof(void **) * max_time);
    for (int k = 0; k < max_time; k++)
    {
        count[k] = 0;
        map_values[k] = NULL;
    }

    map->values = map_values;
    map->count = count;
    map->size = max_time;
    map->type = type;
    return map;
}

void insert(Map *this, int time, void *value)
{
    if (time > this->size - 1)
    {
        if (this->type == PROCESS_M)
        {
            Process *process = (Process *)value;
            nprintf("Erro: o tempo máximo para entrada de processos é de %d, pulando processo %s\n", this->size, process->name);
        }
        else
            nprintf("Erro: o tempo máximo para I/O é de %d, pulando operação\n", this->size);
        return;
    }
    int old_time = this->count[time];
    this->count[time] = old_time + 1;

    // if (this->type == PROCESS_M)
    // {
    //     Process *process = (Process *)value;
    //     nprintf("Inserindo processo %s em t=%d com duração de %d;\n", process->name, time, process->length);
    // }
    // else
    // {
    //     IO *io = (IO *)value;
    //     nprintf("\tInserindo I/O de %s em t=%d;\n", name(io->type), io->start);
    // }

    if (this->values[time] == NULL)
    {
        this->values[time] = malloc(sizeof(void*));
    }
    else
    {
        this->values[time] = realloc(this->values[time], sizeof(void*) * this->count[time]);
    }
    this->values[time][old_time] = value;
}

void** get(Map *this, int time)
{
    return this->values[time];
}

int count(Map *this, int time)
{
    if (time > this->size)
        return 0;
    return this->count[time];
}
