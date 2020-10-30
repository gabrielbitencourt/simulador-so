#include <stdlib.h>
#include <stdio.h>
#include "process.h"

extern FILE *output;
extern int quiet;

StartTimeMap init(int max_time) {
    if (quiet == 0)
	    fprintf(output, "Inicializando tabela de processos com %d espaços de tempo;\n", max_time);
    StartTimeMap map;

    int *count;
    count = malloc(sizeof(int) * max_time);

    Process **map_values;
    map_values = malloc(sizeof(Process*) * max_time);

    for (int k = 0; k < max_time; k++) {
        count[k] = 0;
        map_values[k] = NULL;
    }
    map.values = map_values;
    map.count = count;
    map.size = max_time;
    return map;
}

void insert(StartTimeMap *this, int time, Process value) {
    if (time > this->size - 1) {
        if (quiet == 0)
	        fprintf(output, "Erro: o tempo máximo para entrada de processos é de %d, pulando processo %s\n", this->size, value.name);
        return;
    }
    int old_time = this->count[time];
    this->count[time] = old_time + 1;

    if (quiet == 0)
	    fprintf(output, "Inserindo processo %s em t=%d;\n", value.name, time);
    if (this->values[time] == NULL) {
        this->values[time] = malloc(sizeof(Process));
    }
    else {
        this->values[time] = realloc(this->values[time], sizeof(Process) * this->count[time]);
    }
    this->values[time][old_time] = value;
}

Process* get(StartTimeMap *this, int time) {
    return this->values[time];
}

int count(StartTimeMap *this, int time) {
    if (time > this->size) return 0;
    return this->count[time];
}
