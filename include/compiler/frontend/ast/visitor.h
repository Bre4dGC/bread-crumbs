#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "core/hashmap.h"
#include "core/strings.h"
#include "compiler/frontend/ast.h"

#define NUM_NODE_KINDS 24

typedef void (*visit_func_t)(ast_visitor_t* visitor, node_t* node);

typedef struct ast_visitor_t {
    arena_t* arena;
    hashmap_t* string_table;
    visit_func_t visit_table[NUM_NODE_KINDS];
} ast_visitor_t;

ast_visitor_t* new_ast_visitor(void);
void free_ast_visitor(ast_visitor_t* visitor);
void ast_visit(ast_visitor_t* visitor, ast_t* ast);
