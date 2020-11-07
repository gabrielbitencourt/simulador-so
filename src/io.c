#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "map.h"
#include "process.h"
#include "io.h"

const int DISK_LENGTH = 5;
const int TAPE_LENGTH = 9;
const int PRINTER_LENGTH = 20;

const int LENGTHS[3] = { DISK_LENGTH, TAPE_LENGTH, PRINTER_LENGTH };

char *name(IOType type)
{
    char *name;
    switch (type)
    {
        case DISK:
            name = malloc(sizeof(char) * strlen("Disco"));
            name = "Disco";
            break;
        case TAPE:
            name = malloc(sizeof(char) * strlen("Fita magnética"));
            name = "Fita magnética";
            break;
        case PRINTER:
            name = malloc(sizeof(char) * strlen("Impressora"));
            name = "Impressora";
            break;
    }
    return name;
}

IO *create_io(IOType type, Process *p, int start)
{
    IO *io = malloc(sizeof(IO));
    io->type = type;
    io->process = p;
    io->start = start;
    io->length = LENGTHS[(int) type];
    io->processed = 0;
    io->requested_at = -1;
    return io;
}