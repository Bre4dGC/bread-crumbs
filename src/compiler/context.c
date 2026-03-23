#include <stdlib.h>
#include <string.h>

#include "core/ds/arena.h"
#include "compiler/context.h"
#include "compiler/frontend/semantic/symbol.h"
#include "compiler/middle/ir.h"
#include "compiler/backend/codegen.h"
#include "core/lang/source.h"

compiler_context_t* new_compiler_context(void)
{
    compiler_context_t* ctx = malloc(sizeof(compiler_context_t));
    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(compiler_context_t));

    ctx->phase = COMPILE_PHASE_LEXING;

    ctx->options.debug = false;
    ctx->options.verbose = false;
    ctx->options.repl = false;
    ctx->options.optimization = NONE;

    arena_t* perm_arena  = new_arena(ARENA_PERM_SIZE);
    arena_t* temp_arena  = new_arena(ARENA_TEMP_SIZE);
    arena_t* phase_arena = new_arena(ARENA_PHASE_SIZE);

    if(!perm_arena || !temp_arena || !phase_arena){
        if(perm_arena)  free_arena(perm_arena);
        if(temp_arena)  free_arena(temp_arena);
        if(phase_arena) free_arena(phase_arena);
        free(ctx);
        return NULL;
    }

    ctx->memory.perm_arena = perm_arena;
    ctx->memory.temp_arena = temp_arena;
    ctx->memory.phase_arena = phase_arena;

    ctx->memory.perm_strings = new_string_pool(SP_DEF_CAPACITY);
    ctx->memory.temp_strings = new_string_pool(SP_DEF_CAPACITY);

    ctx->memory.global_idents = new_hashmap();
    ctx->memory.local_idents = new_hashmap();

    ctx->reports = new_report_table(perm_arena);

    ctx->ast = new_ast(ctx->memory.perm_arena);
    ctx->symbols = new_symbol_table(ctx);
    ctx->ir = new_ir();
    ctx->codegen = new_codegen();

    return ctx;
}

void free_compiler_context(compiler_context_t* ctx)
{
    if(!ctx) return;

    if(ctx->symbols) ctx->symbols = NULL;
    if(ctx->ir->instrs) free_ir(ctx->ir);
    if(ctx->codegen->arena || ctx->codegen->string_table) free_codegen(ctx->codegen);
    if(ctx->reports->arena) free_report_table(ctx->reports);

    free_hashmap(ctx->memory.global_idents);
    free_hashmap(ctx->memory.local_idents);

    free_string_pool(&ctx->memory.perm_strings);
    free_string_pool(&ctx->memory.temp_strings);

    if(ctx->src_manager.current) free_source_manager(&ctx->src_manager);

    free(ctx);
}
