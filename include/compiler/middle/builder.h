#pragma once

#include <stddef.h>     // size_t
#include <stdint.h>     // uint8_t

#include "compiler/middle/ir.h"     // ir_t
#include "compiler/context.h"       // compiler_context_t

typedef struct {
    ir_t* ir;
    compiler_context_t* ctx;
} builder_t;

builder_t* new_builder(compiler_context_t* ctx);
void free_builder(builder_t* builder);
ir_t* build_ir(builder_t* builder);
