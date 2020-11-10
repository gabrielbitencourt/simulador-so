#include <stdlib.h>
#include <stdio.h>
#include "map.h"
#include "process.h"

extern FILE *output;

static int pid = 100;

Process *create(char *name, int start, int length, int ion)
{
    Process *p = malloc(sizeof(Process));
    p->name = name;
    p->start = start;
    p->length = length;
    p->processed = 0;
    p->pid = pid + 1;
    p->ppid = 1;
    pid = pid + 1;
    return p;
}