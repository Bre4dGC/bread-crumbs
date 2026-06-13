#include "compiler/backend/codegen.h"   // codegen_t
#include "core/ds/arena.h"               // arena_t

codegen_t* new_codegen(void)
{
    arena_t* arena = new_arena(ARENA_PHASE_SIZE);
    if (!arena) return NULL;

    codegen_t* codegen = arena_alloc(arena, sizeof(codegen_t), alignof(codegen_t));
    if (!codegen) {
        free_arena(arena);
        return NULL;
    }

    codegen->arena = arena;
    codegen->string_table = new_hashmap();
    
    if (!codegen->string_table) {
        free_arena(arena);
        return NULL;
    }

    return codegen;
}

void free_codegen(codegen_t* codegen)
{
    if (!codegen) return;

    if (codegen->string_table) {
        free_hashmap(codegen->string_table);
        codegen->string_table = NULL;
    }

    if (codegen->arena) {
        free_arena(codegen->arena);
        codegen->arena = NULL;
    }
}

void cg_generate(codegen_t* codegen, ast_t* ast)
{
    if (!codegen || !ast) return;
    
    // TODO: Implement code generation
}
