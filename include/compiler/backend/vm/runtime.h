#pragma once

#include <stddef.h>
#include <stdint.h>
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
} vm_runtime_t;

vm_runtime_t* new_vm_runtime(void);
void free_vm_runtime(vm_runtime_t* runtime);