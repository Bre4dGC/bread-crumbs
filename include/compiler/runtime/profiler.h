#pragma once

#include <stddef.h>

typedef struct {
    const char* name;
    size_t call_count;
    size_t total_time;
} profiler_function_t;

typedef struct {
    profiler_function_t* functions;
    size_t function_count;
    size_t function_capacity;
} profiler_t;

profiler_t* new_profiler(void);
void profiler_start(profiler_t* profiler, const char* name);
void profiler_stop(profiler_t* profiler);
void free_profiler(profiler_t* profiler);
