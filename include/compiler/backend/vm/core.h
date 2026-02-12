#pragma once

#include <stddef.h>
#include <stdint.h>

#include "compiler/middle/ir.h"
#include "runtime/memory.h"
#include "runtime/gc.h"

typedef struct {
    ir_t* ir;
    int64_t* stack;
    size_t sp;
    size_t stack_capacity;
    size_t ip;
} vm_stack_t;

typedef struct virtual_machine_t {
    vm_stack_t* stack;
    memory_t* memory;
    garbage_collector_t* gc;
} vm_core_t;

vm_core_t* new_vm_core(void);
void vm_execute(vm_core_t* vm);
void free_vm_core(vm_core_t* vm);