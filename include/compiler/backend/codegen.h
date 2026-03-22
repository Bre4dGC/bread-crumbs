#pragma once

#include "compiler/context.h"           // compiler_context_t
#include "compiler/frontend/ast.h"      // ast_t

typedef struct {
    arena_t* arena;
    hashmap_t* string_table;
} codegen_t;

codegen_t* new_codegen(void);
void free_codegen(codegen_t* codegen);
void cg_generate(codegen_t* codegen, ast_t* ast);
