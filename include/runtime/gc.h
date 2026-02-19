#pragma once

#include <stddef.h>

enum gc_color {
    GC_COLOR_WHITE,
    GC_COLOR_GRAY,
    GC_COLOR_BLACK
};

typedef struct gc_object {
    void* ptr;
    size_t size;
    enum gc_color color;
    struct gc_object* next;
} gc_object_t;

typedef struct {
    gc_object_t* objects;
    size_t total_allocated;
    size_t threshold;
} garbage_collector_t;

garbage_collector_t* new_gc(size_t threshold);
void gc_mark(garbage_collector_t* gc, void* ptr);
void gc_sweep(garbage_collector_t* gc);
void gc_collect(garbage_collector_t* gc);
void free_gc(garbage_collector_t* gc);
