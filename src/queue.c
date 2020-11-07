#include <stdlib.h>
#include <stdio.h>
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