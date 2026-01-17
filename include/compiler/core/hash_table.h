#pragma once
#include <stdint.h>

#include "compiler/core/string_pool.h"

typedef struct hash_table {
    string_t key;
    void* value;
    struct hash_table* next;
} hash_table_t;

uint32_t ht_hash(const char* str);
hash_table_t* new_hashtable(void);
void ht_insert(hash_table_t* table, const char* key, void* value);
void* ht_lookup(hash_table_t* table, const char* key);
void ht_delete(hash_table_t* table, const char* key);
void free_hashtable(hash_table_t* table);
