#pragma once

#include "compiler/backend/vm/core.h"
#include "runtime/memory.h"
#include "runtime/gc.h"
#include "runtime/ffi.h"

typedef struct {
    // TODO: define
} jit_runtime_t;

jit_runtime_t* new_runtime(void);
void free_runtime(jit_runtime_t* runtime);
