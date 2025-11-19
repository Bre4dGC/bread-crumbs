#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "compiler/frontend/lexer.h"

typedef struct astnode astnode_t;

struct node_bin_op {
    astnode_t* left;
    astnode_t* right;
    int operator;
};

struct node_unary_op {
    astnode_t* right;
    int operator;
    bool is_postfix;
};

struct node_var_assign {
    char* name;
    astnode_t* value;
};

struct node_var_ref {
    char* name;
};

struct node_block {
    astnode_t** statements;
    size_t count;
    size_t capacity;
};

struct node_func_call {
    char* name;
    astnode_t** args;
    size_t arg_count;
};

struct node_return_stmt {
    astnode_t* body;
};

struct node_literal {
    enum category_literal type;
    char* value;
};
struct node_var {
    enum category_modifier modif;
    char* name;
    enum category_datatype dtype;
    astnode_t* value;
};

struct node_array {
    astnode_t** elements;
    size_t count;
    size_t capacity;
};

struct node_if {
    astnode_t* condition;
    astnode_t* then_block;
    astnode_t* else_block;
    astnode_t* elif_blocks;
};

struct node_while {
    astnode_t* condition;
    astnode_t* body;
};

struct node_for {
    astnode_t* init;
    astnode_t* condition;
    astnode_t* update;
    astnode_t* body;
};

struct node_func {
    char* name;
    astnode_t** params;
    enum category_datatype return_type;
    astnode_t* body;
    size_t param_count;
};

struct node_case {
    astnode_t* condition;
    astnode_t* body;
};

struct node_match {
    astnode_t* target;
    astnode_t** cases;
    size_t case_count;
    size_t case_capacity;
};

struct node_struct {
    char* name;
    astnode_t** members;
    size_t member_count;
    size_t member_capacity;
};

struct node_enum {
    char* name;
    char** members;
    size_t member_count;
    size_t member_capacity;
};

struct node_union {
    char* name;
    astnode_t** members;
    size_t member_count;
    size_t member_capacity;
};

struct node_trait {
    char* name;
    astnode_t* body;
};

struct node_trycatch {
    astnode_t* try_block;
    astnode_t* catch_block;
    astnode_t* finally_block;
};

struct node_import {
    char* module_name;
};

// struct node_test {
//     char* name;
//     astnode_t* body;
// };

// struct node_fork {
//     enum category_keyword keyw;
//     char* name;
//     astnode_t* body;
// };

// struct node_simulate {
//     enum category_keyword keyw;
//     astnode_t* body;
// };

// struct node_solve {
//     astnode_t** params;
//     size_t param_count;
//     size_t param_capacity;
//     astnode_t* body;
// };

enum node_type {
    NODE_LITERAL,   NODE_BIN_OP,    NODE_EXPR,
    NODE_BLOCK,     NODE_UNARY_OP,  NODE_VAR,
    NODE_IF,        NODE_WHILE,     NODE_FOR,
    NODE_FUNC_CALL, NODE_ASSIGN,
    NODE_VAR_REF,   NODE_ARRAY,     
    NODE_CONTINUE,  NODE_BREAK,     NODE_RETURN,
    NODE_STRUCT,    NODE_ENUM,      NODE_UNION,
    NODE_FUNC,      NODE_MATCH,     NODE_CASE,
    NODE_TRAIT,     NODE_TRYCATCH,  NODE_IMPORT,
    // NODE_TEST,      NODE_FORK,      NODE_SIMULATE,
    // NODE_SOLVE,
};

struct astnode {
    enum node_type type;
    size_t line;
    union {
        struct node_bin_op bin_op;
        struct node_unary_op unary_op;
        struct node_var_assign var_assign;
        struct node_var_ref var_ref;
        struct node_block block;
        struct node_func_call func_call;
        struct node_return_stmt return_stmt;
        struct node_literal literal;

        struct node_var* var_decl;
        struct node_array* array_decl;
        struct node_if* if_stmt;
        struct node_while* while_loop;
        struct node_for* for_loop;
        struct node_func* func_decl;
        struct node_case* match_case;
        struct node_match* match_stmt;
        struct node_struct* struct_decl;
        struct node_enum* enum_decl;
        struct node_union* union_decl;
        struct node_trait* trait_decl;
        struct node_trycatch* trycatch_stmt;
        struct node_import* import_stmt;
        // struct node_test* test_stmt;
        // struct node_fork* fork_stmt;
        // struct node_simulate* simulate_stmt;
        // struct node_solve* solve_stmt;
    };
};

astnode_t* new_ast(enum node_type type);
// int compile_ast(astnode_t* node, struct virtual_machine_t* vm);
void free_ast(astnode_t* node);
