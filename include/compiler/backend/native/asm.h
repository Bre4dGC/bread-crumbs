#pragma once

#include <stddef.h>     // size_t
#include <stdint.h>     // int64_t

#include "core/ds/arena.h"      // arena_t
#include "core/ds/hashmap.h"    // hashmap_t
#include "compiler/middle/ir.h" // ir_t

typedef struct assembler_t {
    arena_t* arena;
    hashmap_t* string_table;
} nat_assembler_t;

nat_assembler_t* new_nat_assembler(void);
void free_nat_assembler(nat_assembler_t* assembler);
void nat_assemble_ir(ir_t* ir, const char* output_path);
