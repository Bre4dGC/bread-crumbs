#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lexer.h"

typedef struct {
    TModifierType modif;
    wchar_t *name;
    TDataType dtype;
    struct ASTNode *literal;
} NodeVar;

typedef struct {
    TDataType dtype;
    struct ASTNode *elements;
} NodeArray;

typedef struct {
    struct ASTNode *cond;
    struct ASTNode *then_block;
    struct ASTNode *else_block;
} NodeIf;

typedef struct {
    struct ASTNode *cond;
    struct ASTNode *body;
} NodeWhile;

typedef struct {
    struct ASTNode *init;
    struct ASTNode *cond;
    struct ASTNode *step;
    struct ASTNode *body;
} NodeFor;

typedef struct {
    TModifierType *modif;
    wchar_t *name;
    struct ASTNode *params;
    TDataType ret_type;
    struct ASTNode *body;
} NodeFunc;

typedef struct {
    struct ASTNode *expr;
    struct ASTNode *body;
    struct ASTNode *end;
} NodeCase;

typedef struct {
    struct ASTNode *cond;
    struct ASTNode *cases;    
} NodeMatch;

typedef struct {
    wchar_t *name;
    struct ASTNode *body;
} NodeStruct;

typedef struct {
    wchar_t *name;
    struct ASTNode *body;
} NodeEnum;

typedef struct {
    wchar_t *name;
    struct ASTNode *body;
} NodeUnion;

typedef struct {
    TModifierType modif;
    wchar_t *name;
    struct ASTNode *body;
} NodeTrait;

typedef struct {
    struct ASTNode *cond;
    struct ASTNode *try_block;
    struct ASTNode *catch_block;
    struct ASTNode *finally_block;
} NodeTryCatch;

typedef struct {
    wchar_t *name;
    struct ASTNode *body;
} NodeTest;

typedef struct {
    TKeywordType keyw;
    wchar_t *name;
    struct ASTNode *body;
} NodeFork;

typedef struct {
    TKeywordType keyw;
    struct ASTNode *body;
} NodeSimulate;

typedef struct {
    struct ASTNode *params;
    struct ASTNode *body;
} NodeSolve;

typedef enum {
    NODE_INT,
    NODE_BIN_OP,
    NODE_VAR,
    NODE_IF,
    NODE_WHILE,
    NODE_FUNC_CALL,
    NODE_ASSIGN,
    NODE_RETURN,
    NODE_BLOCK,
    NODE_EXPR,
    NODE_UNARY_OP,
    NODE_ARRAY,
    NODE_STRUCT, 
    // TODO: add more node types as needed
} NodeType;

typedef enum {
    /* base */
    OP_PUSH,    // push <value>
    OP_POP,     // pop
    OP_DUP,     // duplicate top stack value

    /* arithmetic */
    OP_ADD,     // a + b
    OP_SUB,     // a - b
    OP_MUL,     // a * b
    OP_DIV,     // a / b

    /* logic */
    OP_AND,     // a && b
    OP_OR,      // a || b
    OP_NOT,     // !a

    /* equalation */
    OP_EQ,      // a == b
    OP_NEQ,     // a != b
    OP_LT,      // a < b
    OP_GT,      // a > b

    /* memory */
    OP_STORE,   // store <var_id>
    OP_LOAD,    // load <var_id>
    OP_STORE_GLOB, // store_global <name>
    OP_LOAD_GLOB, // load_global <name>

    /* stream control */
    OP_JUMP,    // jmp <label>
    OP_CALL,    // call <func_id>
    OP_RETURN,  // return
    OP_JUMP_IF, // jmp_if <label>
    OP_JUMP_IFNOT, // jmp_ifnot <label>
    // TODO: add more operations as needed
} OpCode;

typedef struct ASTNode {
    NodeType type;
    int line;

    union {
        struct { 
            int value;
            char *var_name;
        };

        struct literal{
            union {
                /* basic types */
                int int_val;
                unsigned int uint_val;
                float float_val;
                void *void_val;
                bool bool_val;
                char *str_val;
                wchar_t uni_val;
                int **tensor_val;

                /* exact types */
                int8_t int8_val;
                int16_t int16_val;
                int32_t int32_val;
                int64_t int64_val;
                uint8_t uint8_val;
                uint16_t uint16_val;
                uint32_t uint32_val;
                uint64_t uint64_val;
                float float32_val;
                double float64_val;
            };
        } literal;

        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            OpCode code;
        } bin_op;

        struct {
            struct ASTNode *operand;
            OpCode code;
        } unary_op;

        struct {
            wchar_t *name;
            struct ASTNode *value;
        } var_assign;

        struct {
            wchar_t *name;
            TDataType dtype;
            union {
                struct ASTNode *value;
            };
        } var_ref;

        struct {
            struct ASTNode *body;
        } block;

        struct {
            wchar_t *name;
            struct ASTNode *args;
        } func_call;

        NodeVar node_var;
        NodeArray node_array;
        NodeIf node_if;
        NodeWhile node_while;
        NodeFor node_for;
        NodeFunc node_func;
        NodeMatch node_match;
        NodeStruct node_struct;
        NodeEnum node_enum;
        NodeUnion node_union;
        NodeTrait node_trait;
        NodeTryCatch node_trycatch;
        NodeTest node_test;
        NodeFork node_fork;
        NodeSimulate node_simulate;
    };
} ASTNode;

typedef struct {
    uint8_t *code;
    size_t capacity;
    int64_t *stack;
    size_t ssize;   // stack size
    size_t ip;      // instruction pointer 
    size_t sp;      // stack pointer
} VM;

typedef struct Parser {
    Lexer *lexer;
    Token tcurrent;
    Token tnext;
} Parser;

Parser* parser_new(Lexer* lexer);
ASTNode* parse_expr(Parser* parser);
void parser_free(Parser* parser);
void vm_exec(VM* vm);