#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int ALPHA = 5;
const int DISK_LENGTH = 5;
const int TAPE_LENGTH = 9;
const int PRINTER_LENGTH = 20;

typedef enum IOType { DISK = 1, TAPE, PRINTER } IOType;

typedef struct IO {
    IOType type;
    int start;
} IO;

typedef enum ProcessState { NOT_STARTED = 1, READY, WAITING, OCCUPIED, FINISHED } ProcessState;

typedef struct Process {
    char *name;
    IO *io;
    int start;
    int finished;

    int length;
    int processed;


} Process;

typedef struct StartTimeMap {
    Process **values;
    int *count;
} StartTimeMap;

StartTimeMap init(int);
Process* get(StartTimeMap*, int);

StartTimeMap init(int max_time) {
    printf("Initializing start time map with %d time spaces;\n", max_time);
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
    return map;
}

void insert(StartTimeMap *this, int time, Process value) {
    int old_time = this->count[time];
    this->count[time] = old_time + 1;

    printf("inserting %dº process (%s) in time space %d;\n", old_time + 1, value.name, time);
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
    return this->count[time];
}

int main(int argc, char *argv[]) {
    FILE *f = fopen(argv[2], "r");
    if (f == NULL) {
        f = stdin;
    }

    int n = 0, max_time = 200;
    if (f == stdin) printf("Número de processos a serem escalonados: ");
    fscanf(f, "%d", &n);

    StartTimeMap map = init(max_time);

    char str[100];
    for (int i = 0; i < n; i++) {
        int start, length, m;
        if (f == stdin) printf("\n\nNome do processo %d: ", i + 1);
        fscanf(f, "%s", str);

        if (f == stdin) printf("Tempo de início do processo %s (CPU começa em t=0): ", str);
        fscanf(f, "%d", &start);

        if (f == stdin) printf("Tempo de serviço do processo %s: ", str);
        fscanf(f, "%d", &length);

        // if (f == stdin) printf("Número de I/Os do processo %s: ", str);
        // fscanf(f, "%d", &m);
        m = 0;

        IO *ios = malloc(sizeof(IO) * m);
        for (int j = 0; j < m; j++) {
            int type, time;
            if (f == stdin) printf("\n\tTipo da %dª I/O (1: Disco, 2: Fita Magnética, 3: Impressora): ", j + 1);
            fscanf(f, "%d", &type);

            if (f == stdin) printf("\tMomento da %dª operação de IO (em relação ao tempo de serviço do processo): ", j + 1);
            fscanf(f, "%d", &time);

            IO io;
            io.start = time;
            io.type = (IOType) type;
            ios[j] = io;
        }

        size_t str_size = strlen(str);
        char *p_name = malloc(str_size);
        strcpy(p_name, str);
        
        Process p;
        p.name = p_name;
        p.start = start;
        p.length = length;
        p.io = ios;
        p.processed = 0;
        insert(&map, start, p);
    }

    sleep(3);
    int tick = 0, started_processing;
    Process **queue = malloc(sizeof(Process*) * n);
    int processing = -1, free_space = 0;
    int finished = 0;
    while (1) {
        printf("\n\ntick %d\n", tick);
        if (tick < max_time) {
            int starting_n = count(&map, tick);
            if (starting_n > 0) {
                Process *starting = get(&map, tick);
                for (int k = 0; k < starting_n; k++) {
                    printf("adicionando processo %s a fila de espera\n", starting[k].name);
                    queue[free_space] = &starting[k];
                    if (processing == -1) {
                        processing = free_space;
                        started_processing = tick;
                    }
                    free_space = free_space + 1;
                }
            }
        }
        

        if (processing == -1) {
            int next = -1;
            for (int l = (processing + 1) % n; l != processing; l = (l + 1) % n) {
                if (queue[l] != NULL) {
                    next = l;
                }
            }
            if (processing >= 0) {
                printf("saindo de espera e trocando para processo %s\n", queue[next]->name);
            }
            else {
                printf("não há nenhum processo na fila, esperando próximo processo...\n");
            }
            processing = next;
            started_processing = tick;
        }

        if (processing >= 0) {
            // processa por 1 tick
            queue[processing]->processed = queue[processing]->processed + 1;
            printf("processando processo %s, faltam %d ticks para término\n", queue[processing]->name, queue[processing]->length - queue[processing]->processed);

            // verifica se processo finalizou
            if (queue[processing]->length == queue[processing]->processed) {
                printf("processo %s terminado\n", queue[processing]->name);
                queue[processing] = NULL;
                finished = finished + 1;
                if (finished == n) {
                    printf("todos os processos finalizaram\n");
                    break;
                }
                
                // verifica se tem algum processo na fila aguardando alem do atual, se não coloca a cpu em espera
                int next = -1;
                for (int l = (processing + 1) % n; l != processing; l = (l + 1) % n) {
                    if (queue[l] != NULL) {
                        next = l;
                    }
                }
                printf("trocando para processo %s\n", queue[next]->name);
                processing = next;
                started_processing = tick + 1;
            }
            // verifica se ja processou por mais do tempo limite por processo
            if (tick - started_processing >= ALPHA - 1) {
                printf("tempo limite de processamento do processo %s\n", queue[processing]->name);
                // verifica se tem algum processo na fila aguardando alem do atual, se não coloca a cpu em espera
                int next = -1;
                for (int l = (processing + 1) % n; l != processing; l = (l + 1) % n) {
                    if (queue[l] != NULL) {
                        next = l;
                    }
                }
                if (processing >= 0) printf("trocando para processo %s\n", queue[next]->name);
                else printf("não há outro processo na fila, entrando em espera\n");
                processing = next;
                started_processing = tick + 1;
            }
        }
        tick = tick + 1;
        sleep(1);
    }

}
