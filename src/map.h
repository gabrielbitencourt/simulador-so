typedef enum MapType { PROCESS_M, IO_M } MapType;

typedef struct Map {
    void ***values;
    int *count;
    int size;
    MapType type;
} Map;

Map *init(int,MapType);
void insert(Map*, int, void*);
void** get(Map*, int);
int count(Map*, int);