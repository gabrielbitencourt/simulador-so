typedef struct Process
{
    char *name;
    Map *ios;
    int start;
    int length;
    int processed;
    int pid;
    int ppid;
} Process;

Process *create(char*, int, int, int);