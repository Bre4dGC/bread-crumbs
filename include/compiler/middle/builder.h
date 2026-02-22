#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "compiler/middle/ir.h"

typedef struct {
    ir_t* ir;
} builder_t;

builder_t* new_builder(arena_t* arena);
ir_t* build_ir(builder_t* builder);
