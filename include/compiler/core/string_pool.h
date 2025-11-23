#pragma once
#include <stddef.h>

#include "compiler/core/arena_alloc.h"

typedef struct {
    arena_t arena;
    char** ptrs;
    size_t count;
} string_pool_t;

string_pool_t new_string_pool(const size_t capacity);
void free_string_pool(string_pool_t* pool);
char* new_string(string_pool_t* pool, const char* str);
