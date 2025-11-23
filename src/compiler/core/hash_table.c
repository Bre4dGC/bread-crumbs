#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "compiler/core/hash_table.h"

unsigned int ht_hash(const char* str)
{
    unsigned int hash = 5381;
    int c;
    while((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

hash_table_t* new_hashtable()
{
    hash_table_t* table = (hash_table_t*)malloc(sizeof(hash_table_t));
    table->key = 0;
    table->value = NULL;
    table->next = NULL;
    return table;
}

void ht_insert(hash_table_t* table, char* key, void* value)
{
    hash_table_t* entry = table;
    while(entry->next != NULL){
        if(entry->key == key){
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    if(entry->key == key){
        entry->value = value;
        return;
    }
    hash_table_t* new_entry = (hash_table_t*)malloc(sizeof(hash_table_t));
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = NULL;
    entry->next = new_entry;
}

void* ht_lookup(hash_table_t* table, char* key)
{
    while(table != NULL){
        if(table->key == key) return table->value;
        table = table->next;
    }
    return NULL;
}

void ht_delete(hash_table_t* table, char* key)
{
    hash_table_t* entry = table;
    hash_table_t* next = entry->next;

    while(next != NULL){
        if(next->key == key){
            entry->next = next->next;
            free(next);
            return;
        }
        entry = next;
        next = next->next;
    }
}

void free_hashtable(hash_table_t* table)
{
    // recursively free
    if(table->next != NULL) free_hashtable(table->next);
    free(table);
}
