#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "core/hashmap.h"
#include "core/strings.h"

uint32_t hm_hash(const char* str)
{
    uint32_t hash = 2166136261u;
    for(size_t i = 0; str[i] != '\0'; i++){
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;
    }
    return hash;
}

hashmap_t* new_hashmap(void)
{
    hashmap_t* map = (hashmap_t*)malloc(sizeof(hashmap_t));
    if(!map) return NULL;
    map->curr = (bucket_t*)malloc(sizeof(bucket_t));
    if(!map->curr) {
        free(map);
        return NULL;
    }
    map->curr->key = (string_t){NULL, 0, 0};
    map->curr->value = NULL;
    map->next = NULL;
    map->count = 0;
    return map;
}

void hm_insert(hashmap_t* map, const char* key, void* value)
{
    if(!map || !key) return;

    const string_t str = {key, strlen(key), hm_hash(key)};

    if(!map->curr->key.data){
        map->curr->key = str;
        map->curr->value = value;
        return;
    }

    while(map != NULL){
        if(map->curr->key.data && map->curr->key.hash == str.hash){
            if(strcmp(map->curr->key.data, str.data) == 0){
                map->curr->value = value;
                return;
            }
        }
        if(map->next == NULL) break;
        map = map->next;
    }

    hashmap_t* new_entry = (hashmap_t*)malloc(sizeof(hashmap_t));
    if(!new_entry) return;
    new_entry->curr = (bucket_t*)malloc(sizeof(bucket_t));
    if(!new_entry->curr){
        free(new_entry);
        return;
    }
    new_entry->curr->key = str;
    new_entry->curr->value = value;
    new_entry->next = NULL;
    map->count++;
}

void* hm_lookup(hashmap_t* map, const char* key)
{
    if(!map || !key) return NULL;

    const string_t str = {key, strlen(key), hm_hash(key)};

    while(map != NULL){
        if(map->curr->key.data && map->curr->key.hash == str.hash){
            if(strcmp(map->curr->key.data, str.data) == 0){
                return map->curr->value;
            }
        }
        map = map->next;
    }
    return NULL;
}

void hm_delete(hashmap_t* map, const char* key)
{
    if(!map || !key) return;

    const string_t str = {key, strlen(key), hm_hash(key)};

    if(map->curr->key.data && map->curr->key.hash == str.hash){
        if(map->next){
            hashmap_t* next = map->next;
            map->curr->key = next->curr->key;
            map->curr->value = next->curr->value;
            map->next = next->next;
            free(next);
        }
        else {
            map->curr->key.data = NULL;
            map->curr->value = NULL;
        }
        return;
    }

    hashmap_t* prev = map;
    map = map->next;

    while(map != NULL){
        if(map->curr->key.data && map->curr->key.hash == str.hash){
            if(strcmp(map->curr->key.data, str.data) == 0){
                prev->next = map->next;
                free(map);
                return;
            }
        }
        prev = map;
        map = map->next;
    }
}

void free_hashmap(hashmap_t* map)
{
    if(!map) return;
    hashmap_t* curr = map;
    while(curr){
        hashmap_t* next = curr->next;
        free(curr->curr);
        curr = next;
    }
    free(map);
}
