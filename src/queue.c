#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "process.h"
#include "io.h"
#include "queue.h"

Queue *init_queue(int size, QueueType type)
{
    Queue *this = malloc(sizeof(Queue));
    this->size = size;
    this->length = 0;
    this->type = type;
    this->values = malloc(sizeof(void**) * size);
    for (int i = 0; i < size; i++)
    {
        this->values[i] = NULL;
    }
    return this;
}

void push(Queue *this, void* v)
{
    this->values[this->length] = v;
    this->length = this->length + 1;
}

void *pop(Queue *this)
{
    void *current = this->values[0];
    this->values[this->length] = NULL;
    for (int i = 1; i <= this->length; i++) {
        this->values[i - 1] = this->values[i];
    }
    this->length = this->length - 1;
    return current;
}

void *current(Queue* this)
{
    return this->values[0];    
}

char *stringify_queue(Queue *q)
{
    char *str;
    if (q->type == PROCESS_Q)
        str = malloc(3 * sizeof(char) * q->length + 2 * sizeof(char));
    else
        str = malloc(11 * sizeof(char) * q->length + 2 * sizeof(char));

    str[0] = '[';
    int str_index = 1;
    for (int i = 0; i < q->length; i++)
    {
        if (q->type == PROCESS_Q)
        {
            Process *p = q->values[i];
            str[str_index] = p->name[0];
            str[str_index + 1] = ',';
            str[str_index + 2] = ' ';
            str_index = str_index + 3;
        }
        else
        {
            IO *io = q->values[i];
            Process *p = io->process;

            int abrv_len = strlen(abrv(io->type));
            char *io_str = malloc((abrv_len + 6) * sizeof(char));

            sprintf(io_str, "%c - %s, ", p->name[0], abrv(io->type));
            strcpy(&str[str_index], io_str);
            str_index = str_index + abrv_len + 6;
        }
        
    }
    if (q->length > 0)
    {
        str[str_index - 2] = ']';
        str[str_index - 1] = '\0';
    }
    else
    {
        str[str_index] = ']';
        str[str_index + 1] = '\0';
    }
    return str;
}