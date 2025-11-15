#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "compiler/frontend/lexer.h"
#include "compiler/vm.h"

struct ast_node;

struct node_bin_op {
    struct ast_node* left;
    struct ast_node* right;
    enum op_code code;
};

struct node_unary_op {
    struct ast_node* right;
    enum op_code code;
    bool is_postfix;
};

struct node_var_assign {
    char* name;
    struct ast_node* value;
};

struct node_var_ref {
    char* name;
};

struct node_block {
    struct ast_node** statements;
    size_t count;
    size_t capacity;
};

struct node_func_call {
    char* name;
    struct ast_node* *args;
    size_t arg_count;
};

struct node_return_stmt {
    struct ast_node* body;
};

struct node_literal {
    enum category_literal type;
    char* value;
};
struct node_var {
    enum category_modifier modif;
    char* name;
    enum category_datatype dtype;
    struct ast_node* value;
};

struct node_array {
    struct ast_node** elements;
    size_t count;
    size_t capacity;
};

struct node_if {
    struct ast_node* condition;
    struct ast_node* then_block;
    struct ast_node* else_block;
    struct ast_node* elif_blocks;
};

struct node_while {
    struct ast_node* condition;
    struct ast_node* body;
};

struct node_for {
    struct ast_node* init;
    struct ast_node* condition;
    struct ast_node* update;
    struct ast_node* body;
};

struct node_func {
    char* name;
    struct ast_node** params;
    enum category_datatype return_type;
    struct ast_node* body;
    size_t param_count;
};

struct node_case {
    struct ast_node* condition;
    struct ast_node* body;
};

struct node_match {
    struct ast_node* target;
    struct ast_node** cases;
    size_t case_count;
    size_t case_capacity;
};

struct node_struct {
    char* name;
    struct ast_node** members;
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
    struct ast_node** members;
    size_t member_count;
    size_t member_capacity;
};

struct node_trait {
    char* name;
    struct ast_node* body;
};

struct node_trycatch {
    struct ast_node* try_block;
    struct ast_node* catch_block;
    struct ast_node* finally_block;
};

struct node_import {
    char* module_name;
};

struct node_test {
    char* name;
    struct ast_node* body;
};

struct node_fork {
    enum category_keyword keyw;
    char* name;
    struct ast_node* body;
};

struct node_simulate {
    enum category_keyword keyw;
    struct ast_node* body;
};

struct node_solve {
    struct ast_node** params;
    size_t param_count;
    size_t param_capacity;
    struct ast_node* body;
};

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
    NODE_TEST,      NODE_FORK,      NODE_SIMULATE,
    NODE_SOLVE,
};

struct ast_node {
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
        struct node_test* test_stmt;
        struct node_fork* fork_stmt;
        struct node_simulate* simulate_stmt;
        struct node_solve* solve_stmt;
    };
};

struct ast_node* new_ast(enum node_type type);
void compile_ast(struct ast_node* node, struct vm* vm);
void free_ast(struct ast_node* node);
