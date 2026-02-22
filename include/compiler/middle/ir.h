#pragma once
#include <stddef.h>
#include <stdint.h>

#include "core/arena.h"
#include "core/strings.h"
#include "compiler/frontend/semantic/types.h"

enum op_code {
    /* base */
    OP_PUSH,        // push <value>
    OP_POP,         // pop
    OP_DUP,         // duplicate top stack value

    /* arithmetic */
    OP_ADD,         // a + b
    OP_SUB,         // a - b
    OP_MUL,         // a * b
    OP_DIV,         // a / b
    OP_MOD,         // a % b

    /* logic */
    OP_AND,         // a && b
    OP_OR,          // a || b
    OP_NOT,         // !a

    /* equation */
    OP_EQ,          // a == b
    OP_NEQ,         // a != b
    OP_LT,          // a < b
    OP_GT,          // a > b
    OP_LTE,         // a <= b
    OP_GTE,         // a >= b

    /* memory */
    OP_STORE,       // store <var_id>
    OP_LOAD,        // load <var_id>
    OP_STORE_GLOB,  // store_global <name>
    OP_LOAD_GLOB,   // load_global <name>
    OP_ALLOC,       // alloc <size>
    OP_FREE,        // free <address>

    /* stream control */
    OP_LABEL,       // label <name>
    OP_JUMP,        // jmp <label>
    OP_CALL,        // func_call <func_id>
    OP_RETURN,      // return
    OP_JUMP_IF,     // jmp_if <label>
    OP_JUMP_IFNOT,  // jmp_ifnot <label>
};

typedef union {
    int64_t int_value;
    string_t str_value;
    double float_value;
    size_t size_value;
    int64_t addr_value;
    int64_t var_id;
    int64_t func_id;
} ir_data_t;

typedef struct {
    enum op_code op;
    ir_data_t data;
    location_t loc;
} ir_instr_t;

typedef struct {
    ir_instr_t* instrs;
    size_t count;
    size_t capacity;
    arena_t* arena;
} ir_t;

typedef struct {
    char* name;
    ir_t body;
    type_t* return_type;
    size_t locals_count;
} ir_func_t;

ir_t* new_ir(arena_t* arena);
void ir_add_instr(ir_t* ir, enum op_code op, ir_data_t value);
void ir_add_op(ir_t* ir, enum op_code op, int64_t value);
void ir_add_jump(ir_t* ir, int64_t target);
void ir_add_call(ir_t* ir, int64_t func_id);
void ir_add_return(ir_t* ir);
void ir_add_jump_if(ir_t* ir, int64_t target);
void ir_add_jump_ifnot(ir_t* ir, int64_t target);
