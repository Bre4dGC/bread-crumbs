#pragma once

#include "core/ds/arena.h"      // arena_t
#include "core/ds/strings.h"    // string_pool_t
#include "core/ds/hashmap.h"    // hashmap_t

#include "core/lang/diagnostic.h"   // report_table_t
#include "core/lang/source.h"       // source_manager_t

#include "compiler/frontend/ast.h" // ast_t

typedef struct compiler_context compiler_context_t;

#include "compiler/frontend/semantic/symbol.h" // symbol_table_t
#include "compiler/backend/codegen.h"   // ir_t, codegen_t

enum compile_phase {
    COMPILE_PHASE_START = 0,
    COMPILE_PHASE_LEXING,
    COMPILE_PHASE_PARSING,
    COMPILE_PHASE_ANALYSIS,
    COMPILE_PHASE_CODEGEN,
    COMPILE_PHASE_EXECUTION,
    COMPILE_PHASE_FINISH
};

typedef struct {
    bool debug;
    bool verbose;
    bool repl;
    enum {NONE, SOFT, HARD} optimization;
} compiler_option_t;

typedef struct {
    arena_t* perm_arena;     // long-lived allocations
    arena_t* phase_arena;    // allocations during a phase
    arena_t* temp_arena;     // temporary allocations

    string_pool_t perm_strings;
    string_pool_t temp_strings;

    hashmap_t global_idents;
    hashmap_t local_idents;
} compiler_memory_t;

struct compiler_context {
    enum compile_phase phase;

    source_manager_t src_manager;
    compiler_option_t options;
    compiler_memory_t memory;

    report_table_t reports;

    ast_t ast_root;
    symbol_table_t* symbols;
    ir_t ir;
    codegen_t codegen;
};

compiler_context_t* new_compiler_context(void);
void free_compiler_context(compiler_context_t* ctx);
