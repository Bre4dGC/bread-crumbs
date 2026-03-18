#pragma once

#include "compiler/middle/ir.h"             // ir_t
#include "compiler/backend/vm/compiler.h"   // compiler_t
#include "runtime/memory.h"                 // memory_t
#include "runtime/gc.h"                     // garbage_collector_t
#include "runtime/ffi.h"                    // ffi_registry_t

typedef struct {
    ir_t* ir;
    compiler_t* compiler;
    memory_t* memory;
    garbage_collector_t* gc;
    ffi_registry_t* ffi;
} jit_compiler_t;

jit_compiler_t* new_jit_compiler(void);
void free_jit_compiler(jit_compiler_t* compiler);