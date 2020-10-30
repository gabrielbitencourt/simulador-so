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
    int size;
} StartTimeMap;

StartTimeMap init(int);
void insert(StartTimeMap*, int, Process);
Process* get(StartTimeMap*, int);
int count(StartTimeMap*, int);
int has_io(Process*, int);