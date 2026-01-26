#include <stdlib.h>
#include <string.h>

#include "compiler/context.h"
#include "compiler/core/diagnostic.h"
#include "compiler/frontend/semantic.h"

compiler_memory_t* new_compiler_memory(void)
{
    compiler_memory_t* mem = (compiler_memory_t*)malloc(sizeof(compiler_memory_t));
    if(!mem) return NULL;

    return mem;
}

void free_compiler_memory(compiler_memory_t* mem)
{
    if(!mem) return;
}

compiler_context_t* new_compiler_context(void)
{
    compiler_context_t* ctx = (compiler_context_t*)malloc(sizeof(compiler_context_t));
    if(!ctx) return NULL;

    return ctx;
}

void free_compiler_context(compiler_context_t* ctx)
{
    if(!ctx) return;
}
