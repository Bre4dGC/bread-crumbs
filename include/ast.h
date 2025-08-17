#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"
#include "vm.h"

struct node_var{
    enum modifier_category modif;
    char *name;
    enum datatype_category dtype;
    struct ast_node *literal;
};

struct node_array{
    enum datatype_category dtype;
    struct ast_node *elements;
};

struct node_if{
    struct ast_node *cond;
    struct ast_node *then_block;
    struct ast_node *else_block;
};

struct node_while{
    struct ast_node *cond;
    struct ast_node *body;
};

struct node_for{
    struct ast_node *init;
    struct ast_node *cond;
    struct ast_node *step;
    struct ast_node *body;
};

struct node_func{
    enum modifier_category *modif;
    char *name;
    struct ast_node *params;
    enum datatype_category ret_type;
    struct ast_node *body;
};

struct node_case{
    struct ast_node *expr;
    struct ast_node *body;
    struct ast_node *end;
};

struct node_match{
    struct ast_node *cond;
    struct ast_node *cases;    
};

struct node_struct{
    char *name;
    struct ast_node *body;
};

struct node_enum{
    char *name;
    struct ast_node *body;
};

struct node_union{
    char *name;
    struct ast_node *body;
};

struct node_trait{
    enum modifier_category modif;
    char *name;
    struct ast_node *body;
};

struct node_trycatch{
    struct ast_node *cond;
    struct ast_node *try_block;
    struct ast_node *catch_block;
    struct ast_node *finally_block;
};

struct node_import{
    char *name;
};

struct node_test{
    char *name;
    struct ast_node *body;
};

struct node_fork{
    enum keyword_category keyw;
    char *name;
    struct ast_node *body;
};

struct node_simulate{
    enum keyword_category keyw;
    struct ast_node *body;
};

struct node_solve{
    struct ast_node *params;
    struct ast_node *body;
};

enum node_type{
    NODE_BIN_OP,    NODE_EXPR,      NODE_BLOCK, 
    NODE_UNARY_OP,  NODE_VAR,       NODE_IF,    
    NODE_WHILE,     NODE_FUNC_CALL, NODE_ASSIGN,
    NODE_RETURN,    NODE_ARRAY,     NODE_STRUCT,   
    NODE_FOR,       NODE_FUNC,      NODE_MATCH,     
    NODE_ENUM,      NODE_UNION,     NODE_TRAIT,     
    NODE_TRYCATCH,  NODE_IMPORT,    NODE_TEST,      
    NODE_FORK,      NODE_SIMULATE,  NODE_SOLVE,
    // TODO: add more node types as needed
};

struct ast_node {
    enum node_type type;
    size_t line;
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
                char uni_val;
                int64_t **tensor_val;

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
        };

        struct bin_op{
            struct ast_node *left;
            struct ast_node *right;
            enum op_code code;
        };

        struct unary_op{
            struct ast_node *operand;
            enum op_code code;
        };

        struct var_assign{
            char *name;
            struct ast_node *value;
        };

        struct var_ref{
            char *name;
            enum datatype_category dtype;
            union {
                struct ast_node *value;
            };
        };

        struct block{
            struct ast_node *body;
        };

        struct func_call{
            char *name;
            struct ast_node *args;
        };

        struct return_stmt{
            struct ast_node *body;
        };

        struct node_var *var_decl;
        struct node_array *array_decl;
        struct node_if *if_stmt;
        struct node_while *while_loop;
        struct node_for *for_loop;
        struct node_func *func_decl;
        struct node_case *case_stmt;
        struct node_match *match_stmt;
        struct node_struct *struct_decl;
        struct node_enum *enum_decl;
        struct node_union *union_decl;
        struct node_trait *trait_decl;
        struct node_trycatch *trycatch_stmt;
        struct node_import *import_stmt;
        struct node_test *test_stmt;
        struct node_fork *fork_stmt;
        struct node_simulate *simulate_stmt;
        struct node_solve *solve_stmt;
        struct node_bin_op *bin_op;
        struct node_unary_op *unary_op;
        struct node_var_assign *var_assign;
        struct node_var_ref *var_ref;
        struct node_block *block;
        struct node_func_call *func_call;
        struct node_return_stmt *return_stmt;
    };
};

struct ast_node* new_ast(enum node_type type);
void free_ast(struct ast_node* node);