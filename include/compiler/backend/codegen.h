#pragma once

#include "core/ds/arena.h"          // arena_t
#include "core/ds/strings.h"        // string_t
#include "core/ds/hashmap.h"        // hashmap_t
#include "compiler/frontend/ast.h"  // ast_t
#include "compiler/middle/ir.h"     // ir_t

typedef struct {
    arena_t* arena;
    hashmap_t* string_table;
} codegen_t;

codegen_t* new_codegen(void);
void free_codegen(codegen_t* codegen);
void cg_generate(codegen_t* codegen, ast_t* ast);
