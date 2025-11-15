#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "compiler/frontend/ast.h"
#include "compiler/middle/types.h"

enum symbol_kind {
    SYMBOL_VAR,
    SYMBOL_CONST,
    SYMBOL_FUNC,
    SYMBOL_PARAM,
    SYMBOL_STRUCT,
    SYMBOL_ENUM,
    SYMBOL_UNION,
    SYMBOL_TYPE,
};

enum symbol_flags {
    SYM_FLAG_NONE     = 0,
    SYM_FLAG_USED     = 1 << 0,  // symbol was referenced
    SYM_FLAG_ASSIGNED = 1 << 1,  // variable was assigned
    SYM_FLAG_GLOBAL   = 1 << 2,  // global scope
    SYM_FLAG_EXTERN   = 1 << 3,  // external linkage
    SYM_FLAG_STATIC   = 1 << 4,  // static storage
};
    
enum scope_kind {
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_STRUCT,
};

struct symbol {
    char* name;
    enum symbol_kind kind;    
    struct type* type;
    uint32_t flags;

    struct ast_node* decl_node;
    
    size_t line;
    size_t column;
    
    struct scope* scope; // for nested scopes
    struct symbol* next; // hash table linkage
};

struct scope {
    struct scope* parent;    // enclosing scope
    struct symbol** symbols; // hash table of symbols    
    struct ast_node* owner;
    enum scope_kind kind;
    
    size_t capacity;
    size_t count;
};

struct symbol_table {
    struct scope* current;
    struct scope* global;
    struct scope** scopes;
    size_t scope_count;
    size_t scope_capacity;
};

struct symbol_table* new_symbol_table(void);
struct symbol* lookup_symbol(struct symbol_table* st, const char* name);
struct symbol* define_symbol(struct symbol_table* st, const char* name, const enum symbol_kind kind, struct type* type, struct ast_node* decl_node);
struct scope* push_scope(struct symbol_table* st, int scope_kind, struct ast_node* owner);
bool is_scope_symbol_exist(struct symbol_table* st, const char* name);
void pop_scope(struct symbol_table* st);
void free_scope(struct scope* scope);
void free_symbol_table(struct symbol_table* st);
