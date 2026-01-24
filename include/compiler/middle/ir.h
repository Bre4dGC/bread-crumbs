#pragma once
#include <stddef.h>
#include <stdint.h>

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
    OP_JUMP,        // jmp <label>
    OP_CALL,        // func_call <func_id>
    OP_RETURN,      // return
    OP_JUMP_IF,     // jmp_if <label>
    OP_JUMP_IFNOT,  // jmp_ifnot <label>
};

typedef struct {
    enum op_code op;
    int64_t value;
} ir_instr_t;

typedef struct {
    ir_instr_t* instructions;
    size_t count;
    size_t capacity;
} ir_t;

ir_t* new_ir(void);
void free_ir(ir_t* ir);
void ir_add_instruction(ir_t* ir, enum op_code op, int64_t value);
