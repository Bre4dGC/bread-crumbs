#pragma once

#include <stdbool.h>    // bool
#include <stddef.h>     // size_t

#include "compiler/context.h"       // compiler_context_t
#include "compiler/frontend/ast.h"  // node_t
#include "compiler/frontend/semantic/symbol.h"  // symbol_table_t, symbol_t

enum semantic_phase {
    PHASE_DECLARE, // register symbols
    PHASE_RESOLVE, // resolve types and references
    PHASE_CHECK    // type checking and validation
};

typedef struct {
    enum semantic_phase phase;
    symbol_table_t* symbols;
    symbol_t* current_function;
    int loop_depth;

    compiler_context_t* ctx;
} semantic_t;

semantic_t* new_semantic(compiler_context_t* ctx);
bool analyze_ast(semantic_t* ctx, node_t* ast);
void free_semantic(semantic_t* ctx);
