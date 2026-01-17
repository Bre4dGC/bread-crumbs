#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/core/hash_table.h"
#include "compiler/core/string_pool.h"

#include "compiler/frontend/ast.h"
#include "compiler/frontend/types.h"

enum symbol_kind {
    SYMBOL_VAR,
    SYMBOL_CONST,
    SYMBOL_FUNC,
    SYMBOL_PARAM,
    SYMBOL_STRUCT,
    SYMBOL_ENUM,
    SYMBOL_ENUM_VARIANT,
    SYMBOL_UNION,
    SYMBOL_TYPE_ALIAS,
    SYMBOL_BUILTIN_TYPE,
    SYMBOL_MODULE,
    SYMBOL_GENERIC,
};

enum symbol_flags {
    SYM_FLAG_NONE     = 0,
    SYM_FLAG_USED     = 1 << 0,  // symbol was referenced
    SYM_FLAG_ASSIGNED = 1 << 1,  // variable was assigned
    SYM_FLAG_GLOBAL   = 1 << 2,  // global scope
    SYM_FLAG_EXTERN   = 1 << 3,  // external linkage
    SYM_FLAG_STATIC   = 1 << 4,  // static storage
    SYM_FLAG_MUTABLE  = 1 << 5,  // 
    SYM_FLAG_PRIVATE  = 1 << 8,  // private visibility
    SYM_FLAG_PUBLIC   = 1 << 9,  // public visibility
};
    
enum scope_kind {
    SCOPE_GLOBAL,
    SCOPE_MODULE,
    SCOPE_FUNCTION,
    SCOPE_BLOCK,
    SCOPE_STRUCT,
    SCOPE_ENUM,
    SCOPE_UNION,
    SCOPE_SWITCH,
    SCOPE_LOOP,
};

typedef struct scope scope_t;
typedef struct symbol symbol_t;

struct scope {
    scope_t* parent;
    scope_t* first_child;
    scope_t* next_sibling;

    enum scope_kind kind;
    node_t* owner;
    int depth;

    hash_table_t* symbols;
    size_t count;
};

struct symbol {
    char* name;
    location_t loc;

    enum symbol_kind kind;
    enum symbol_flags flags;
    
    struct type* type;
    struct type* declared_type;

    node_t* decl_node;
    node_t* init_node;
    
    symbol_t* next_in_scope;
    symbol_t* shadowed_symbol;
    symbol_t* overload_next;

    scope_t* scope;
};

typedef struct {
    scope_t* global;
    scope_t* current;
    size_t scope_count;
    size_t scope_capacity;
    arena_t* arena;
    string_pool_t* string_pool;
} symbol_table_t;

symbol_table_t* new_symbol_table(arena_t* arena, string_pool_t* string_pool);
symbol_t* lookup_symbol(symbol_table_t* st, const char* name);
symbol_t* define_symbol(symbol_table_t* st, const char* name, const enum symbol_kind kind, struct type* type, node_t* decl_node);
scope_t* push_scope(symbol_table_t* st, int scope_kind, node_t* owner);
void pop_scope(symbol_table_t* st);

bool is_scope_symbol_exist(symbol_table_t* st, const char* name);

void free_scope(scope_t* scope);
void free_symbol_table(symbol_table_t* st);
