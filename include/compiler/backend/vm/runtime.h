#pragma once

#include <stddef.h>
#include <stdint.h>

#include "compiler/backend/vm/core.h"
#include "runtime/memory.h"
#include "runtime/gc.h"
#include "runtime/ffi.h"

typedef struct {
    // TODO: define
} vm_runtime_t;

vm_runtime_t* new_vm_runtime(void);
void free_vm_runtime(vm_runtime_t* runtime);
