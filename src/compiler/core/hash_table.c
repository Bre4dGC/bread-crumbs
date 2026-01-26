#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/hash_table.h"

uint32_t ht_hash(const char* str)
{
    uint32_t hash = 2166136261u;
    for(size_t i = 0; str[i] != '\0'; i++){
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;
    }
    return hash;
}

hash_table_t* new_hashtable(void)
{
    hash_table_t* table = (hash_table_t*)malloc(sizeof(hash_table_t));
    if(!table) return NULL;
    table->key.data = NULL;
    table->value = NULL;
    table->next = NULL;
    return table;
}

#include <stdio.h>

void ht_insert(hash_table_t* table, const char* key, void* value)
{
    if(!table || !key) return;

    const string_t str = {key, strlen(key), ht_hash(key)};

    if(table->key.data == NULL){
        table->key = str;
        table->value = value;
        return;
    }

    while(table != NULL){
        if(table->key.data && table->key.hash == str.hash){
            if(strcmp(table->key.data, str.data) == 0){
                table->value = value;
            }
            return;
        }
        if(table->next == NULL) break;
        table = table->next;
    }

    hash_table_t* new_entry = (hash_table_t*)malloc(sizeof(hash_table_t));
    if(!new_entry) return;
    new_entry->key = str;
    new_entry->value = value;
    new_entry->next = NULL;
    table->next = new_entry;
}

void* ht_lookup(hash_table_t* table, const char* key)
{
    if(!table || !key) return NULL;

    const string_t str = {key, strlen(key), ht_hash(key)};

    while(table != NULL){
        if(table->key.data && table->key.hash == str.hash){
            if(strcmp(table->key.data, str.data) == 0){
                return table->value;
            }
        }
        table = table->next;
    }
    return NULL;
}

void ht_delete(hash_table_t* table, const char* key)
{
    if(!table || !key) return;

    const string_t str = {key, strlen(key), ht_hash(key)};

    if(table->key.data && table->key.hash == str.hash){
        if(table->next){
            hash_table_t* next = table->next;
            table->key = next->key;
            table->value = next->value;
            table->next = next->next;
            free(next);
        }
        else {
            table->key.data = NULL;
            table->value = NULL;
        }
        return;
    }

    hash_table_t* entry = table;
    hash_table_t* next = entry->next;

    while(next != NULL){
        if(next->key.data && next->key.hash == str.hash){
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
    if(!table) return;
    if(table->next != NULL) free_hashtable(table->next);
    free(table);
}
