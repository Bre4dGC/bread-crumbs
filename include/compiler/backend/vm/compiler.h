#pragma once

#include "core/ds/arena.h"      // arena_t
#include "core/ds/hashmap.h"    // hashmap_t
#include "compiler/middle/ir.h" // ir_t

typedef struct compiler_t {
    arena_t* arena;
    hashmap_t* string_table;
} vm_compiler_t;

vm_compiler_t* new_vm_compiler(void);
void free_vm_compiler(vm_compiler_t* compiler);
ir_t* vm_compile_ir(ir_t* ir);
