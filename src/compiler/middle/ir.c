#include "compiler/middle/ir.h" // ir_t, op_code

ir_t* new_ir(void)
{
    // TODO: implement
    return NULL;
}

void free_ir(ir_t* ir)
{
    // TODO: implement
}

ir_func_t* new_ir_func(const char* name, size_t param_count, size_t local_count)
{
    // TODO: implement
    return NULL;
}

void free_ir_func(ir_func_t* func)
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
