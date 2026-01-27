#pragma once

#include <stddef.h>
#include <stdint.h>

#include "compiler/middle/ir.h"
#include "compiler/runtime/memory.h"
#include "compiler/runtime/gc.h"

typedef struct {
    ir_t* ir;
    int64_t* stack;
    size_t sp;
    size_t stack_capacity;
    size_t ip;
} vm_stack_t;

typedef struct {
    vm_stack_t stack;
    memory_t* memory;
    garbage_collector_t* gc;
} virtual_machine_t;

void vm_execute(virtual_machine_t* vm);
void free_vm(virtual_machine_t* vm);