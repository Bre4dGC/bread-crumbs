#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "compiler/frontend/ast.h"
#include "compiler/frontend/symbol.h"
#include "compiler/frontend/types.h"

enum semantic_phase {
    PHASE_DECLARE, // register symbols
    PHASE_RESOLVE, // resolve types and references  
    PHASE_CHECK    // type checking and validation
};

typedef struct {
    symbol_table_t* symbols;
    enum semantic_phase phase;

    symbol_t* current_function;
    int loop_depth;

    report_t** errors;
    size_t errors_count;
} semantic_context_t;

semantic_context_t* new_semantic_context(void);
void free_semantic_context(semantic_context_t* ctx);
bool analyze_ast(semantic_context_t* ctx, astnode_t* ast);
