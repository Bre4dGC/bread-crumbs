#pragma once

#include "core/ds/arena.h"          // arena_t
#include "core/ds/hashmap.h"        // hashmap_t
#include "compiler/frontend/ast.h"  // ast_t, node_t

#define NUM_NODE_KINDS 24

typedef struct ast_visitor ast_visitor_t;

typedef void (*visit_func_t)(ast_visitor_t* visitor, node_t* node);

typedef struct ast_visitor {
    arena_t* arena;
    hashmap_t* string_table;
    visit_func_t visit_table[NUM_NODE_KINDS];
} ast_visitor_t;

ast_visitor_t* new_ast_visitor(arena_t* arena);
void free_ast_visitor(ast_visitor_t* visitor);
void ast_visit(ast_visitor_t* visitor, ast_t* ast);
