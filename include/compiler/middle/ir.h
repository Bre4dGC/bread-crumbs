#pragma once

#include <stddef.h> // size_t
#include <stdbool.h>

#include <llvm-c/Core.h>

#include "compiler/frontend/ast.h"  // ast_t, node_t, DT_*
#include "core/ds/strings.h"        // string_t
#include "core/ds/arena.h"          // arena_t

typedef struct compiler_context compiler_context_t;
typedef struct symbol_table symbol_table_t;

typedef struct ir_value ir_value_t;
typedef struct ir_block ir_block_t;
typedef struct ir_func ir_func_t;
typedef struct ir_module ir_module_t;
typedef struct ir_instr ir_instr_t;

enum ir_instr_op {
    IR_CONST,
    IR_ALLOCA,
    IR_LOAD,
    IR_STORE,
    IR_BINOP,
    IR_UNOP,
    IR_CALL,
    IR_RET,
    IR_RET_VOID,
    IR_BR,
    IR_COND_BR,
};

enum ir_binop_op {
    IR_BINOP_ADD,
    IR_BINOP_SUB,
    IR_BINOP_MUL,
    IR_BINOP_DIV,
    IR_BINOP_MOD,
    IR_BINOP_EQ,
    IR_BINOP_NEQ,
    IR_BINOP_LT,
    IR_BINOP_GT,
    IR_BINOP_LTE,
    IR_BINOP_GTE,
    IR_BINOP_AND,
    IR_BINOP_OR,
};

enum ir_unop_op {
    IR_UNOP_NEG,
    IR_UNOP_NOT,
};

enum ir_val_kind {
    IR_VAL_CONST_INT,
    IR_VAL_CONST_FLOAT,
    IR_VAL_CONST_BOOL,
    IR_VAL_CONST_STR,
    IR_VAL_CONST_NULL,
    IR_VAL_REG,
    IR_VAL_PARAM,
    IR_VAL_ALLOCA,
    IR_VAL_GLOBAL,
    IR_VAL_FUNC,
};

struct ir_value {
    enum ir_val_kind kind;
    int type;              // DT_* datatype
    string_t name;         // variable name
    LLVMValueRef llvm_val; // cached LLVM value
    union {
        int i;
        double d;
        bool b;
        string_t str;
    };
    ir_value_t* next;
};

#define IR_MAX_OPERANDS 16

struct ir_instr {
    enum ir_instr_op op;
    ir_value_t* result;
    ir_value_t* operands[IR_MAX_OPERANDS];
    size_t operand_count;
    union {
        enum ir_binop_op binop_op;
        enum ir_unop_op unop_op;
        ir_block_t* dest_block;
        struct { ir_block_t* then_block; ir_block_t* else_block; } cond_br;
    };
    ir_instr_t* next;
};

struct ir_block {
    string_t name;
    ir_instr_t* first;
    ir_instr_t* last;
    ir_block_t* next;
    bool terminated;
};

struct ir_func {
    string_t name;
    int return_type;         // DT_*
    ir_value_t** params;
    size_t param_count;
    ir_block_t* first_block;
    ir_block_t* last_block;
    size_t block_count;
    ir_func_t* next;
};

struct ir_module {
    string_t name;
    ir_func_t* first_func;
    ir_func_t* last_func;
    size_t global_count;
    ir_value_t** globals;
};

struct ir {
    arena_t* arena;
    ir_module_t* module;
    size_t next_reg;
};

typedef struct ir ir_t;

ir_t* new_ir(void);
void free_ir(ir_t* ir);

ir_module_t* ir_module_create(ir_t* ir, const char* name);
ir_func_t* ir_func_create(ir_t* ir, ir_module_t* mod, const char* name,
                          int return_type, ir_value_t** params, size_t param_count);
ir_value_t* new_value(arena_t* arena, enum ir_val_kind kind, int type);
ir_block_t* ir_block_create(ir_t* ir, ir_func_t* func, const char* name);
ir_t* ir_build(ast_t* ast, symbol_table_t* symbols, compiler_context_t* ctx);

void ir_global(ir_t* ir, const char* name, int type);
