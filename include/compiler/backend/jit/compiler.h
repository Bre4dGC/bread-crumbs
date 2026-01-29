#pragma once

#include "compiler/middle/ir.h"
#include "compiler/backend/vm/compiler.h"
#include "compiler/runtime/memory.h"
#include "compiler/runtime/gc.h"
#include "compiler/runtime/ffi.h"

typedef struct {
    ir_t* ir;
    compiler_t* compiler;
    memory_t* memory;
    garbage_collector_t* gc;
    ffi_registry_t* ffi;
} jit_compiler_t;

jit_compiler_t* new_jit_compiler(void);
void free_jit_compiler(jit_compiler_t* compiler);