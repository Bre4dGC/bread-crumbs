// Intermediate Representation for a stack-based virtual machine
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

    /* logic */
    OP_AND,         // a && b
    OP_OR,          // a || b
    OP_NOT,         // !a

    /* equation */
    OP_EQ,          // a == b
    OP_NEQ,         // a != b
    OP_LT,          // a < b
    OP_GT,          // a > b

    /* memory */
    OP_STORE,       // store <var_id>
    OP_LOAD,        // load <var_id>
    OP_STORE_GLOB,  // store_global <name>
    OP_LOAD_GLOB,   // load_global <name>

    /* stream control */
    OP_JUMP,        // jmp <label>
    OP_CALL,        // call <func_id>
    OP_RETURN,      // return
    OP_JUMP_IF,     // jmp_if <label>
    OP_JUMP_IFNOT,  // jmp_ifnot <label>
};

typedef struct {
    uint8_t* code;
    size_t code_size;
    size_t capacity;

    int64_t* stack;
    size_t sp;
    size_t stack_capacity;

    size_t ip;    
} virtual_machine_t;


void vm_execute(virtual_machine_t* vm);
