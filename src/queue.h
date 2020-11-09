typedef enum QueueType { PROCESS_Q, IO_Q } QueueType;

typedef struct Queue {
    void **values;
    int size;
    int length;
    QueueType type;
} Queue;

Queue *init_queue(int, QueueType);
void push(Queue*, void*);
void *pop(Queue*);
void *current(Queue*);
char *stringify_queue(Queue*);