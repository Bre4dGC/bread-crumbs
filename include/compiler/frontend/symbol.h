#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "compiler/frontend/ast.h"
#include "compiler/frontend/types.h"

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

typedef struct scope scope_t;
typedef struct symbol symbol_t;

struct scope {
    struct scope* parent;
    enum scope_kind kind;
    astnode_t* owner;
    symbol_t** symbols;
    size_t count;
    size_t capacity;
};

struct symbol {
    char* name;

    enum symbol_kind kind;
    enum symbol_flags flags;
    
    struct type* type;
    astnode_t* decl_node;
    
    size_t line;
    size_t column;

    struct scope_t* scope;
    struct symbol* next; // for hash table chaining
};

typedef struct {
    scope_t* current;
    scope_t* global;
    scope_t** scopes;

    size_t scope_count;
    size_t scope_capacity;
} symbol_table_t;

symbol_table_t* new_symbol_table(void);
symbol_t* lookup_symbol(symbol_table_t* st, const char* name);
symbol_t* define_symbol(symbol_table_t* st, const char* name, const enum symbol_kind kind, struct type* type, astnode_t* decl_node);
scope_t* push_scope(symbol_table_t* st, int scope_kind, astnode_t* owner);
bool is_scope_symbol_exist(symbol_table_t* st, const char* name);
void pop_scope(symbol_table_t* st);
void free_scope(scope_t* scope);
void free_symbol_table(symbol_table_t* st);
