#pragma once
#include <stddef.h>
#include <stdint.h>

#include "compiler/middle/ir.h"

typedef struct {
    ir_t* ir;
} builder_t;

builder_t* new_builder(void);
void free_builder(builder_t* builder);
ir_t* build_ir(builder_t* builder);