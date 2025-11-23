#pragma once

typedef struct hash_table {
    char* key;
    void* value;
    struct hash_table* next;
} hash_table_t;

hash_table_t* new_hashtable();
unsigned int ht_hash(const char* str);
void ht_insert(hash_table_t* table, char* key, void* value);
void* ht_lookup(hash_table_t* table, char* key);
void ht_delete(hash_table_t* table, char* key);
void free_hashtable(hash_table_t* table);