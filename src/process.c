#include <stdlib.h>
#include <stdio.h>
#include "map.h"
#include "process.h"

extern int quiet;
extern FILE *output;

Process *create(char *name, int start, int length, int ion)
{
    Process *p = malloc(sizeof(Process));
    p->name = name;
    p->start = start;
    p->length = length;
    p->processed = 0;
    return p;
}