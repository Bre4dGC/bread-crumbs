#pragma once

#include "core/arena.h"
#include "core/strings.h"
#include "core/hashmap.h"
#include "compiler/frontend/ast.h"
#include "compiler/middle/ir.h"

typedef struct {
    arena_t* arena;
    hashmap_t* string_table;
} codegen_t;

codegen_t* new_codegen(void);
void free_codegen(codegen_t* codegen);
void cg_generate(codegen_t* codegen, ast_t* ast);
