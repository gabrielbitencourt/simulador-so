struct Process;

typedef enum IOType
{
    DISK = 0,
    TAPE,
    PRINTER
} IOType;

typedef struct IO
{
    IOType type;
    struct Process *process;
    int start;
    int length;
    int processed;
    int requested_at;
} IO;

char *name(IOType);
IO *create_io(IOType, struct Process*, int);