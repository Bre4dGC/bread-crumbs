#include <stdlib.h>

#include "compiler/frontend/symbol.h"
#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/core/hash_table.h"

#define INITIAL_SCOPE_CAPACITY 16
#define SYMBOL_TABLE_SIZE 64

static symbol_t* lookup_in_scope(scope_t* scope, const char* name);

scope_t* new_scope(int kind, node_t* owner)
{
    scope_t* scope = (scope_t*)malloc(sizeof(scope_t));
    if(!scope) return NULL;
    
    scope->parent = NULL;
    scope->first_child = NULL;
    scope->next_sibling = NULL;

    scope->parent = NULL;
    scope->kind = kind;
    scope->owner = owner;
    scope->count = 0;
    scope->depth = 0;
    
    scope->symbols = new_hashtable();
    if(!scope->symbols){
        free_scope(scope);
        return NULL;
    }
    
    return scope;
}

symbol_table_t* new_symbol_table(arena_t* arena, string_pool_t* string_pool)
{
    symbol_table_t* st = (symbol_table_t*)arena_alloc(arena, sizeof(symbol_table_t), alignof(symbol_table_t));
    if(!st) return NULL;
    
    st->global = new_scope(SCOPE_GLOBAL, NULL);
    if(!st->global){
        free_symbol_table(st);
        return NULL;
    }
    
    st->current = st->global;
    st->scope_capacity = INITIAL_SCOPE_CAPACITY;
    st->scope_count = 1;
    st->arena = arena;
    st->string_pool = string_pool;
    return st;
}

scope_t* push_scope(symbol_table_t* st, int scope_kind, node_t* owner)
{
    if(!st) return NULL;
    
    scope_t* scope = new_scope(scope_kind, owner);
    if(!scope) return NULL;

    scope->parent = st->current;
    scope->depth = scope->parent ? (scope->parent->depth + 1) : 0;

    // link into parent's child list (singly-linked)
    if(scope->parent){
        if(!scope->parent->first_child){
            scope->parent->first_child = scope;
        }
        else {
            scope_t* sib = scope->parent->first_child;
            while(sib->next_sibling) sib = sib->next_sibling;
            sib->next_sibling = scope;
        }
    }

    st->current = scope;
    st->scope_count += 1;
    return scope;
}

void pop_scope(symbol_table_t* st)
{
    if(!st || !st->current || st->current == st->global) return;
    scope_t* dead = st->current;
    st->current = dead->parent;
    free_scope(dead);
}

scope_t* current_scope(symbol_table_t* st)
{
    return st ? st->current : NULL;
}

symbol_t* define_symbol(symbol_table_t* st, const char* name, const enum symbol_kind kind, type_t* type, node_t* decl_node)
{
    if(!st || !name) return NULL;
    
    scope_t* scope = st->current;
    if(!scope) return NULL;
    if(lookup_in_scope(scope, name) != NULL){
        return NULL;
    }
    
    // Create new symbol
    symbol_t* sym = (symbol_t*)arena_alloc(st->arena, sizeof(symbol_t), alignof(symbol_t));
    if(!sym) return NULL;
    
    // Intern name so it remains valid for the table's lifetime
    string_t interned = new_string(st->string_pool, name);
    if(!interned.data) return NULL;
    sym->name = (char*)interned.data;
    
    sym->kind = kind;
    sym->type = type;
    sym->declared_type = NULL;
    sym->flags = SYM_FLAG_NONE;
    sym->decl_node = decl_node;
    sym->init_node = NULL;
    sym->loc = decl_node ? decl_node->loc : (location_t){1, 1};
    sym->scope = scope;
    sym->next_in_scope = NULL;
    sym->shadowed_symbol = NULL;
    sym->overload_next = NULL;

    if(scope == st->global){
        sym->flags |= SYM_FLAG_GLOBAL;
    }

    ht_insert(scope->symbols, sym->name, sym);
    scope->count += 1;
    
    return sym;
}

static symbol_t* lookup_in_scope(scope_t* scope, const char* name)
{
    if(!scope || !name) return NULL;
    
    return (symbol_t*)ht_lookup(scope->symbols, name);
}

symbol_t* lookup_symbol(symbol_table_t* st, const char* name)
{
    if(!st || !name) return NULL;
    
    scope_t* scope = st->current;
    while(scope){
        symbol_t* sym = lookup_in_scope(scope, name);
        if(sym) return sym;
        scope = scope->parent;
    }
    
    return NULL;
}

bool is_scope_symbol_exist(symbol_table_t* st, const char* name)
{
    if(!st || !name) return false;
    return lookup_in_scope(st->current, name) != NULL;
}

bool is_symbol_mutable(const symbol_t* sym)
{
    if(!sym) return false;
    return sym->kind != SYMBOL_CONST;
}

bool is_symbol_initialized(const symbol_t* sym)
{
    if(!sym) return false;
    return (sym->flags & SYM_FLAG_ASSIGNED) != 0;
}

void free_scope(scope_t* scope)
{
    if(!scope) return;
    
    if(scope->symbols) free_hashtable(scope->symbols);
    scope->symbols = NULL;
    scope->parent = NULL;
    scope->first_child = NULL;
    scope->next_sibling = NULL;

    free(scope);
    scope = NULL;    
}

void free_symbol_table(symbol_table_t* st)
{
    if(!st) return;
    // st itself is arena-allocated; free only heap-owned scopes.
    if(st->global){
        free_scope(st->global);
    }
    st->global = NULL;
    st->current = NULL;
}
