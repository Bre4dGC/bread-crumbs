#pragma once
#include <stddef.h>
#include "compiler/core/arena_alloc.h"

typedef struct {
    arena_t arena;
} string_pool_t;

string_pool_t new_string_pool(size_t size);
void free_string_pool(string_pool_t* pool);
const char* string_pool_intern(string_pool_t* pool, const char* str);
const char* string_pool_get(string_pool_t* pool, size_t index);
