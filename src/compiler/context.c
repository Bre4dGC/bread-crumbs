#include <stdlib.h>

#include "compiler/context.h"

compiler_memory_t* new_compiler_memory(void)
{
    compiler_memory_t* mem = malloc(sizeof(compiler_memory_t));
    if(!mem) return NULL;

    return mem;
}

void free_compiler_memory(compiler_memory_t* mem)
{
    if(!mem) return;
}

compiler_context_t* new_compiler_context(void)
{
    compiler_context_t* ctx = malloc(sizeof(compiler_context_t));
    if(!ctx) return NULL;

    return ctx;
}

void free_compiler_context(compiler_context_t* ctx)
{
    if(!ctx) return;
}
