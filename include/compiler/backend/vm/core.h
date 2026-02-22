#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "compiler/middle/ir.h"
#include "compiler/backend/vm/memory.h"
#include "runtime/memory.h"
#include "runtime/gc.h"

typedef struct {
    ir_t* ir;
    int64_t* s;
    size_t sp;  // stack pointer
    size_t ip;  // instruction pointer
    size_t pc;  // program counter
} vm_stack_t;

typedef struct {
    vm_stack_t* stack;
    ir_instr_t* code;
    size_t count;
    bool running;
} vm_t;

vm_t* new_vm(arena_t* arena);
void vm_execute(vm_t* vm);
void free_vm(vm_t* vm);