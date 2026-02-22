#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "core/hashmap.h"
#include "compiler/middle/ir.h"

typedef struct {
    // TODO: define
} nat_asm_t;

nat_asm_t* new_nat_asm(void);
void free_nat_asm(nat_asm_t* assembler);
void nat_assemble_ir(ir_t* ir, const char* output_path);
