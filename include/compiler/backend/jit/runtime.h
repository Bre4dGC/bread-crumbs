#pragma once

#include "compiler/backend/vm/core.h"       // vm_core_t
#include "compiler/backend/vm/compiler.h"   // vm_compiler_t
#include "runtime/memory.h"                 // memory_t
#include "runtime/gc.h"                     // garbage_collector_t
#include "runtime/ffi.h"                    // ffi_registry_t

typedef struct jit_runtime_t {
    memory_t* memory;
    garbage_collector_t* gc;
    ffi_registry_t* ffi_registry;
    vm_core_t* vm_core;
    vm_compiler_t* vm_compiler;
} jit_runtime_t;

jit_runtime_t* new_runtime(void);
void free_runtime(jit_runtime_t* runtime);
