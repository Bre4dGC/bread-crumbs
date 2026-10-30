#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/lang/diagnostic.h"
#include "core/lang/source.h"
#include "core/ds/arena.h"
#include "compiler/context.h"
#include "compiler/frontend/semantic/symbol.h"
#include "compiler/middle/ir.h"
#include "compiler/backend/codegen.h"
#include "compiler/backend/native.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser.h"
#include "compiler/frontend/semantic.h"

compiler_context_t* new_compiler_context(const char* filename)
{
    compiler_context_t* ctx = malloc(sizeof(compiler_context_t));
    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(compiler_context_t));

    ctx->phase = COMPILE_PHASE_START;

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

    ctx->src_manager.string_pool = &ctx->memory.perm_strings;
    ctx->src_manager.current = source_manager_load(&ctx->src_manager, filename);
    if(!ctx->src_manager.current){
        /* Keep diagnostics usable even when the source cannot be loaded. */
        ctx->src_manager.current = new_source(filename);
    }

    ctx->reports = new_report_table(perm_arena);
    if(!ctx->reports){
        free_compiler_context(ctx);
        return NULL;
    }

    init_tokens();

    ctx->ast = new_ast(ctx->memory.perm_arena);
    ctx->symbols = new_symbol_table(ctx);

    ctx->ir = new_ir();
    ctx->codegen = codegen_new();

    return ctx;
}

void free_compiler_context(compiler_context_t* ctx)
{
    if(!ctx) return;

    if(ctx->codegen) codegen_free(ctx->codegen);
    if(ctx->ir) free_ir(ctx->ir);

    if(ctx->symbols){
        free_symbol_table(ctx->symbols);
        ctx->symbols = NULL;
    }
    if(ctx->reports) free_report_table(ctx->reports);

    free_source_manager(&ctx->src_manager);

    free_hashmap(ctx->memory.global_idents);
    free_hashmap(ctx->memory.local_idents);

    free_string_pool(&ctx->memory.perm_strings);
    free_string_pool(&ctx->memory.temp_strings);

    if(ctx->memory.perm_arena)  free_arena(ctx->memory.perm_arena);
    if(ctx->memory.temp_arena)  free_arena(ctx->memory.temp_arena);
    if(ctx->memory.phase_arena) free_arena(ctx->memory.phase_arena);

    free(ctx);
}

bool compile_source(compiler_context_t* ctx)
{
    if(!ctx) return false;

    /* Phase: LEXING + PARSING */
    ctx->phase = COMPILE_PHASE_LEXING;
    lexer_t* lexer = new_lexer(ctx);
    if(!lexer){
        add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_UNEXP_EOF, new_location());
        return false;
    }

    ctx->phase = COMPILE_PHASE_PARSING;
    parser_t* parser = new_parser(ctx, lexer);
    if(!parser){
        add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_UNEXP_EOF, new_location());
        return false;
    }

    ast_t* ast = parse_program(parser);
    if(!ast){
        if(ctx->reports->count == 0)
            add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_INVAL_EXPR, new_location());
        return false;
    }
    ctx->ast = ast;

    /* Phase: ANALYSIS */
    ctx->phase = COMPILE_PHASE_ANALYSIS;
    semantic_t* sem = new_semantic(ctx);
    if(!sem){
        add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_INVAL_EXPR, new_location());
        return false;
    }

    bool ok = analyze_ast(sem, ast->nodes);
    /* Transfer symbol table ownership to ctx before freeing sem */
    symbol_table_t* old_symbols = ctx->symbols;
    ctx->symbols = sem->symbols;
    sem->symbols = NULL;
    free_semantic(sem);

    if(old_symbols) free_symbol_table(old_symbols);

    if(!ok){
        /* semantic analysis already reported the specific error */
        if(ctx->reports->count == 0)
            add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_TYPE_MISMATCH, new_location());
        return false;
    }

    /* require a main function for execution */
    if(!lookup_symbol(ctx->symbols, "main")){
        add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_UNDEC_FUNC, new_location());
        return false;
    }

    /* Phase: CODEGEN */
    ctx->phase = COMPILE_PHASE_CODEGEN;

    ir_t* ir = ir_build(ast, ctx->symbols, ctx);
    if(!ir){
        if(ctx->reports->count == 0)
            add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_INVAL_EXPR, new_location());
        return false;
    }
    ctx->ir = ir;

    if(!codegen_emit(ctx->codegen, ir)){
        add_report(ctx->reports, ctx->src_manager.current, SEV_ERR, ERR_INVAL_EXPR, new_location());
        return false;
    }

    ctx->phase = COMPILE_PHASE_EXECUTION;
    return true;
}

bool compile_to_object(compiler_context_t* ctx, const char* output_path)
{
    if(!ctx || !output_path) return false;
    if(!compile_source(ctx)) return false;

    native_t* nat = native_create();
    if(!nat) return false;
    bool ok = native_emit_object(nat, ctx->codegen, output_path);
    native_free(nat);
    return ok;
}

static bool resolve_runtime_lib(char* out, size_t size)
{
    const char* env = getenv("CRUM_RUNTIME_LIB");
    if(env && env[0] && access(env, R_OK) == 0){
        snprintf(out, size, "%s", env);
        return true;
    }

#ifdef BRT_LIB_DIR
    snprintf(out, size, "%s/libbrc.a", BRT_LIB_DIR);
    if(access(out, R_OK) == 0) return true;
#endif

    const char* candidates[] = {
        "lib/libbrc.a",
        "/usr/local/lib/libbrc.a",
    };

    for(size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++){
        snprintf(out, size, "%s", candidates[i]);
        if(access(out, R_OK) == 0) return true;
    }

    return false;
}

bool compile_to_executable(compiler_context_t* ctx, const char* output_path)
{
    if(!ctx || !output_path) return false;

    /* compile to a temporary object file first */
    char tmp_obj[] = "/tmp/brc_obj_XXXXXX";
    int fd = mkstemp(tmp_obj);
    if(fd < 0) return false;
    close(fd);

    if(!compile_to_object(ctx, tmp_obj)){
        unlink(tmp_obj);
        return false;
    }

    char runtime_lib[1024];
    bool has_runtime = resolve_runtime_lib(runtime_lib, sizeof(runtime_lib));

    /* link with clang into an executable */
    char cmd[4096];
    int n;
    if(has_runtime){
        n = snprintf(cmd, sizeof(cmd), "clang \"%s\" \"%s\" -o \"%s\"", tmp_obj, runtime_lib, output_path);
    }
    else {
        n = snprintf(cmd, sizeof(cmd), "clang \"%s\" -o \"%s\"", tmp_obj, output_path);
    }
    if(n < 0 || (size_t)n >= sizeof(cmd)){
        unlink(tmp_obj);
        return false;
    }

    int ret = system(cmd);
    unlink(tmp_obj);
    return ret == 0;
}
