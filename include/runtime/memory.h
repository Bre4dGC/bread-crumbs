#pragma once

#include <stddef.h>

enum memory_region {
    MEM_REGION_HEAP,
    MEM_REGION_STACK,
    MEM_REGION_STATIC,
    MEM_REGION_CODE,
};

typedef struct {
    void* ptr;
    size_t size;
    enum memory_region region;
} memory_block_t;

typedef struct {
    memory_block_t* blocks;
    size_t num_blocks;
    size_t total_allocated;
} memory_t;

memory_t* new_memory_manager(void);
void* mem_alloc(memory_t* mem, size_t size, enum memory_region region);
void mem_free(memory_t* mem, void* ptr);
void free_memory_manager(memory_t* mem);
