#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "compiler/middle/ir.h"

ir_t* new_ir(void)
{
    // TODO: implement
}

void free_ir(ir_t* ir)
{
    // TODO: implement
}

void ir_add_op(ir_t* ir, enum op_code op, int64_t value)
{
    // TODO: implement
}

void ir_add_jump(ir_t* ir, int64_t target)
{
    // TODO: implement
}

void ir_add_call(ir_t* ir, int64_t func_id)
{
    // TODO: implement
}

void ir_add_return(ir_t* ir)
{
    // TODO: implement
}

void ir_add_jump_if(ir_t* ir, int64_t target)
{
    // TODO: implement
}

void ir_add_jump_ifnot(ir_t* ir, int64_t target)
{
    // TODO: implement
}
