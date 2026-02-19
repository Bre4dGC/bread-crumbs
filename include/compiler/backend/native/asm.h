#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "core/hashmap.h"
#include "compiler/middle/ir.h"

typedef struct assembler_t {
    arena_t* arena;
    hashmap_t* string_table;
} nat_assembler_t;

nat_assembler_t* new_nat_assembler(void);
void free_nat_assembler(nat_assembler_t* assembler);
void nat_assemble_ir(ir_t* ir, const char* output_path);
