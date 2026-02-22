#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "compiler/middle/ir.h"

ir_t* new_ir(arena_t* arena)
{
    ir_t* ir = arena_alloc(arena, sizeof(ir_t), alignof(ir_t));
    ir->instrs = NULL;
    ir->count = 0;
    ir->capacity = 0;
    ir->arena = arena;
    return ir;
}

void ir_add_instr(ir_t* ir, enum op_code op, ir_data_t value)
{
    if(ir->count >= ir->capacity){
        size_t new_capacity = ir->capacity == 0 ? 8 : ir->capacity * 2;
        ir_instr_t* new_instrs = arena_alloc(ir->arena, sizeof(ir_instr_t) * new_capacity, alignof(ir_instr_t));
        if(!new_instrs) return;
        if(ir->instrs) memcpy(new_instrs, ir->instrs, sizeof(ir_instr_t) * ir->count);
        ir->instrs = new_instrs;
        ir->capacity = new_capacity;
    }

    ir->instrs[ir->count].op = op;
    ir->instrs[ir->count].data = value;
    ir->instrs[ir->count].loc = (location_t){0};
    ir->count++;
}

void ir_add_op(ir_t* ir, enum op_code op, int64_t value)
{
    ir_data_t data;
    data.int_value = value;
    ir_add_instr(ir, op, data);
}

void ir_add_jump(ir_t* ir, int64_t target)
{
    ir_data_t data;
    data.int_value = target;
    ir_add_instr(ir, OP_JUMP, data);
}

void ir_add_call(ir_t* ir, int64_t func_id)
{
    ir_data_t data;
    data.int_value = func_id;
    ir_add_instr(ir, OP_CALL, data);
}

void ir_add_return(ir_t* ir)
{
    ir_add_instr(ir, OP_RETURN, (ir_data_t){0});
}

void ir_add_jump_if(ir_t* ir, int64_t target)
{
    ir_data_t data;
    data.int_value = target;
    ir_add_instr(ir, OP_JUMP_IF, data);
}

void ir_add_jump_ifnot(ir_t* ir, int64_t target)
{
    ir_data_t data;
    data.int_value = target;
    ir_add_instr(ir, OP_JUMP_IFNOT, data);
}
