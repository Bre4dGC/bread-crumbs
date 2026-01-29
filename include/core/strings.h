#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"

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
string_t new_nstring(string_pool_t* pool, const char* str, const size_t length);
string_t new_string(string_pool_t* pool, const char* str);
void free_string_pool(string_pool_t* pool);
