#pragma once
#include <stddef.h>
#include <stdbool.h>

#include "core/arena.h"
#include "core/strings.h"
#include "core/diagnostic.h"

typedef struct node node_t;

typedef struct {
    node_t** elems;
    size_t count;
    size_t capacity;
} nodes_t;

struct node_binop {
    node_t* left;
    node_t* right;
    int operator;
#ifdef DEBUG
    const char* lit;
#endif
};

struct node_unaryop {
    node_t* right;
    int operator;
    bool is_postfix;
#ifdef DEBUG
    const char* lit;
#endif
};

struct node_var_assign {
    string_t name;
    node_t* value;
};

struct node_var_ref {
    string_t name;
};

struct node_block {
    nodes_t statement;
};

struct node_func_call {
    string_t name;
    nodes_t args;
};

struct node_return {
    node_t* body;
};

struct node_literal {
    int type;
    string_t value;
};

struct node_var {
    int modif;
    string_t name;
    int dtype;
    node_t* value;
};

struct node_array {
    node_t** elements;
    size_t count;
    size_t capacity;
};

struct node_if {
    node_t* condition;
    node_t* then_block;
    node_t* elif_blocks;
    node_t* else_block;
};

struct node_while {
    node_t* condition;
    node_t* body;
};

struct node_for {
    node_t* init;
    node_t* condition;
    node_t* update;
    node_t* body;
};

struct node_param {
    string_t name;
    bool is_variadic;
    int dtype;
};

struct node_func {
    string_t name;
    nodes_t param_decl;
    int return_type;
    node_t* body;
};

struct node_case {
    node_t* condition;
    node_t* body;
};

struct node_match {
    node_t* target;
    nodes_t block;
};

struct node_struct {
    string_t name;
    nodes_t member;
};

struct node_member {
    string_t name;
    node_t* value;
};

struct node_enum {
    string_t name;
    nodes_t member;
};

struct node_trait {
    string_t name;
    node_t* body;
};

struct node_impl {
    string_t trait_name;
    string_t struct_name;
    node_t* body;
};

struct node_trycatch {
    node_t* try_block;
    node_t* catch_block;
    node_t* finally_block;
};

struct node_type {
    string_t name;
    node_t* body;
};

struct node_module {
    string_t name;
    node_t* body;
};

struct node_import {
    string_t* modules;
    size_t count;
    size_t capacity;
};

struct node_special {
    int type;
    string_t content;
};

enum node_kind {
    NODE_LITERAL, NODE_BINOP,   NODE_EXPR,
    NODE_BLOCK,   NODE_UNARYOP, NODE_VAR,
    NODE_CALL,    NODE_ASSIGN,  NODE_REF,
    NODE_MEMBER,  NODE_PARAM,

    NODE_IF,      NODE_WHILE,   NODE_FOR,
    NODE_FUNC,    NODE_MATCH,   NODE_CASE,
    NODE_STRUCT,  NODE_ENUM,    NODE_ARRAY,
    NODE_RETURN,  NODE_BREAK,   NODE_CONTINUE,

    NODE_TYPE,    NODE_IMPORT,  NODE_MODULE,
    NODE_TRAIT,   NODE_IMPL,    NODE_TRYCATCH,

    NODE_NAMEOF,  NODE_TYPEOF,
};

struct node {
    enum node_kind kind;
    location_t loc;
    size_t length;

    union {
        struct node_binop*      binop;
        struct node_unaryop*    unaryop;
        struct node_var_assign* var_assign;
        struct node_var_ref*    var_ref;
        struct node_block*      block;
        struct node_func_call*  func_call;
        struct node_literal*    lit;

        struct node_var*    var_decl;
        struct node_array*  array_decl;
        struct node_param*  param_decl;
        struct node_member* member_decl;
        struct node_func*   func_decl;
        struct node_type*   type_decl;
        struct node_struct* struct_decl;
        struct node_enum*   enum_decl;
        struct node_trait*  trait_decl;
        struct node_module* module_decl;
        struct node_import* import_decl;
        struct node_impl*   impl_decl;

        struct node_if*       if_stmt;
        struct node_while*    while_stmt;
        struct node_for*      for_stmt;
        struct node_case*     case_stmt;
        struct node_match*    match_stmt;
        struct node_trycatch* trycatch_stmt;
        struct node_special*  special_stmt;
        struct node_return*   return_stmt;
    };
};

typedef struct {
    node_t* nodes;
    size_t count;
} ast_t;

node_t* new_node(arena_t* arena, enum node_kind kind);
void free_ast(arena_t* ast);
