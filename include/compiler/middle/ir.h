#pragma once

#include <stddef.h> // size_t

#include "compiler/frontend/ast.h"  // ast_t

typedef struct {
    ast_t ast;
} ir_t;

ir_t* new_ir(void);
void free_ir(ir_t* ir);
