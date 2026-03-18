#pragma once

#include <stddef.h>     // size_t
#include <stdint.h>     // int64_t

#include "core/ds/arena.h"      // arena_t
#include "core/ds/hashmap.h"    // hashmap_t
#include "compiler/middle/ir.h" // ir_t

typedef struct compiler_t {
    arena_t* arena;
    hashmap_t* string_table;
} nat_compiler_t;

nat_compiler_t* new_nat_compiler(void);
void free_nat_compiler(nat_compiler_t* compiler);
ir_t* nat_compile_ir(ir_t* ir);
