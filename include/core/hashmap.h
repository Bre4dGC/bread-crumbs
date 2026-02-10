#pragma once
#include <stdint.h>

#include "core/strings.h"

typedef struct hashmap_t {
    string_t key;
    void* value;
    struct hashmap_t* next;
    size_t count;
} hashmap_t;

uint32_t hm_hash(const char* str);
hashmap_t* new_hashmap(void);
void hm_insert(hashmap_t* map, const char* key, void* value);
void* hm_lookup(hashmap_t* map, const char* key);
void hm_delete(hashmap_t* map, const char* key);
void free_hashmap(hashmap_t* map);
