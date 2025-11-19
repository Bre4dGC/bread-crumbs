#pragma once

typedef struct {
    void* value;
    int key;
    int size;
    int capacity;
} hash_table_t;