typedef enum CPUState { FAILED = -1, WAITED = 1, PROCESSED } CPUState;

typedef struct CPU {
    CPUState state;
    int tick;
    int started_processing;
    int processes_number;
    int free_space;
    int processing;
    int finished;
    int queue_size;
    Process **queue;
} CPU;

void start(CPU*, int);
CPUState tick(CPU*);
int get_next_process(CPU*);
int new_process(CPU*, Process*);
void swap_to_process(CPU*, int, int);
void start_waiting(CPU*);