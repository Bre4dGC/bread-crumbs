#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "compiler/frontend/ast.h"
#include "compiler/middle/types.h"
#include "compiler/middle/symbol.h"

enum semantic_phase {
    PHASE_DECLARE, // register symbols
    PHASE_RESOLVE, // resolve types and references  
    PHASE_CHECK    // type checking and validation
};

struct semantic_context {
    struct symbol_table* symbols;
    enum semantic_phase phase;

    struct symbol* current_function;
    int loop_depth;

    struct error** errors;
    size_t errors_count;
};

struct semantic_context* new_semantic_context(void);
void free_semantic_context(struct semantic_context* ctx);
bool analyze_ast(struct semantic_context* ctx, struct ast_node* ast);
