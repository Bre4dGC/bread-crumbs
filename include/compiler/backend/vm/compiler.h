#pragma once

#include "core/arena.h"
#include "core/hashmap.h"
#include "core/strings.h"
#include "compiler/middle/ir.h"

typedef struct compiler_t {
    arena_t* arena;
    hashmap_t* string_table;
} vm_compiler_t;

vm_compiler_t* new_vm_compiler(void);
void free_vm_compiler(vm_compiler_t* compiler);
ir_t* vm_compile_ir(ir_t* ir);