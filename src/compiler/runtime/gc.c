#include "compiler/runtime/gc.h"

garbage_collector_t* new_gc(size_t threshold)
{
    // TODO: Implement
}

void gc_mark(garbage_collector_t* gc, void* ptr)
{
    // TODO: Implement
}

void gc_sweep(garbage_collector_t* gc)
{
    // TODO: Implement
}

void gc_collect(garbage_collector_t* gc)
{
    // TODO: Implement
}

void free_gc(garbage_collector_t* gc)
{
    // TODO: Implement
}
