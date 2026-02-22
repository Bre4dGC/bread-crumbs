#pragma once

#include "compiler/middle/ir.h"
#include "compiler/backend/vm/core.h"
#include "runtime/memory.h"
#include "runtime/gc.h"
#include "runtime/ffi.h"

typedef struct {
    // TODO: define
} jit_compiler_t;

jit_compiler_t* new_jit_compiler(void);
void free_jit_compiler(jit_compiler_t* compiler);