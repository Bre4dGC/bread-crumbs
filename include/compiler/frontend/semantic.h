#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/semantic/symbol.h"

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

    arena_t* arena;
    report_table_t* reports;
} semantic_t;

semantic_t* new_semantic(arena_t* arena, string_pool_t* string_pool, report_table_t* reports);
bool analyze_ast(semantic_t* ctx, node_t* ast);
void free_semantic(semantic_t* ctx);
