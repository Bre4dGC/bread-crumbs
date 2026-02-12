#pragma once

#include "compiler/middle/ir.h"
#include "runtime/memory.h"
#include "runtime/gc.h"
#include "runtime/ffi.h"
#include "compiler/backend/vm/core.h"
#include "compiler/backend/vm/compiler.h"

typedef struct jit_runtime_t {
    memory_t* memory;
    garbage_collector_t* gc;
    ffi_registry_t* ffi_registry;
    vm_core_t* vm_core;
    vm_compiler_t* vm_compiler;
} jit_runtime_t;

jit_runtime_t* new_runtime(void);
void free_runtime(jit_runtime_t* runtime);