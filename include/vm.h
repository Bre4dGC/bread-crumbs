#pragma once
#include <stdint.h>
#include <stdlib.h>

enum op_code{
    /* base */
    OPCD_PUSH,        // push <value>
    OPCD_POP,         // pop
    OPCD_DUP,         // duplicate top stack value

    /* arithmetic */
    OPCD_ADD,         // a + b
    OPCD_SUB,         // a - b
    OPCD_MUL,         // a * b
    OPCD_DIV,         // a / b

    /* logic */
    OPCD_AND,         // a && b
    OPCD_OR,          // a || b
    OPCD_NOT,         // !a

    /* equalation */
    OPCD_EQ,          // a == b
    OPCD_NEQ,         // a != b
    OPCD_LT,          // a < b
    OPCD_GT,          // a > b

    /* memory */
    OPCD_STORE,       // store <var_id>
    OPCD_LOAD,        // load <var_id>
    OPCD_STORE_GLOB,  // store_global <name>
    OPCD_LOAD_GLOB,   // load_global <name>

    /* stream control */
    OPCD_JUMP,        // jmp <label>
    OPCD_CALL,        // call <func_id>
    OPCD_RETURN,      // return
    OPCD_JUMP_IF,     // jmp_if <label>
    OPCD_JUMP_IFNOT,  // jmp_ifnot <label>
    // TODO: add more operations as needed
};

struct vm{
    uint8_t *code;
    size_t capacity;
    int64_t *stack;
    size_t ssize;   // stack size
    size_t ip;      // instruction pointer 
    size_t sp;      // stack pointer
};

void vm_exec(struct vm* vm);