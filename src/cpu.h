typedef enum CPUState { FAILED = -1, WAITED = 1, PROCESSED } CPUState;

typedef struct CPU {
    CPUState state;
    int tick;
    Process *executing;
    int started_processing;
    Queue hp_queue;
    Queue lp_queue;
    Queue disk_queue;
    Queue tape_queue;
    Queue printer_queue;
} CPU;

void start(CPU*, int);
CPUState tick(CPU*);
void new_process(CPU*, Process*);
void execute(CPU*, Process*);

bool requests_io(CPU*, Process*);
void execute_ios(CPU*);