typedef enum ProcessState
{
    NOT_STARTED = 1,
    READY,
    WAITING,
    OCCUPIED,
    FINISHED
} ProcessState;

typedef struct Process
{
    char *name;
    Map *ios;
    int start;
    int length;
    int processed;
} Process;

Process *create(char*, int, int, int);