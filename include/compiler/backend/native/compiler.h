#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "core/hashmap.h"
#include "compiler/middle/ir.h"

typedef struct compiler_t {
    arena_t* arena;
    hashmap_t* string_table;
} nat_compiler_t;

nat_compiler_t* new_nat_compiler(void);
void free_nat_compiler(nat_compiler_t* compiler);
ir_t* nat_compile_ir(ir_t* ir);
