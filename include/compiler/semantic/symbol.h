#pragma once
#include <stdint.h>

#include "types.h"

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
    
    size_t capacity;
    size_t count;
    
    enum {
        SCOPE_GLOBAL,
        SCOPE_FUNCTION,
        SCOPE_BLOCK,
        SCOPE_STRUCT,
    } kind;
};

struct symbol_table {
    struct scope* current;
    struct scope* global;
    struct scope** scopes;
    size_t scope_count;
    size_t scope_capacity;
};
