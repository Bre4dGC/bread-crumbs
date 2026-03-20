#pragma once

#include <stddef.h>     // size_t
#include <stdint.h>     // uint32_t

#include "core/ds/arena.h"  // arena_t

#define SP_DEF_CAPACITY 64

typedef struct {
    const char* data;
    size_t length;
    uint32_t hash;
} string_t;

typedef struct {
    arena_t* arena;
    string_t* elements;
    size_t count;
    size_t capacity;
} string_pool_t;

string_pool_t new_string_pool(const size_t capacity);
void free_string_pool(string_pool_t* pool);

string_t new_string_n(string_pool_t* pool, const char* str, const size_t length);
string_t new_string(string_pool_t* pool, const char* str);
