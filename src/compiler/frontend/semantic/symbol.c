#include <stdlib.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "core/hashmap.h"
#include "compiler/frontend/semantic/symbol.h"

#define INITIAL_SCOPE_CAPACITY 16
#define SYMBOL_TABLE_SIZE 64

static symbol_t* lookup_in_scope(scope_t* scope, const char* name);

scope_t* new_scope(arena_t* arena, int kind, node_t* owner)
{
    if(!arena) return NULL;
    
    scope_t* scope = arena_alloc(arena, sizeof(scope_t), alignof(scope_t));
    if(!scope) return NULL;

    scope->parent = NULL;
    scope->first_child = NULL;
    scope->next_sibling = NULL;

    scope->parent = NULL;
    scope->kind = kind;
    scope->owner = owner;
    scope->count = 0;
    scope->depth = 0;

    scope->symbols = new_hashmap();
    if(!scope->symbols) return NULL;

    return scope;
}

symbol_table_t* new_symbol_table(arena_t* arena, string_pool_t* string_pool)
{
    symbol_table_t* st = arena_alloc(arena, sizeof(symbol_table_t), alignof(symbol_table_t));
    if(!st) return NULL;

    st->arena = arena;
    st->string_pool = string_pool;
    
    st->global = new_scope(st->arena, SCOPE_GLOBAL, NULL);
    if(!st->global) return NULL;

    st->current = st->global;
    st->scope_capacity = INITIAL_SCOPE_CAPACITY;
    st->scope_count = 1;
    
    return st;
}

scope_t* push_scope(symbol_table_t* st, int scope_kind, node_t* owner)
{
    if(!st) return NULL;

    scope_t* scope = new_scope(st->arena, scope_kind, owner);
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

    if(dead->parent && dead->parent->first_child){
        if(dead->parent->first_child == dead){
            dead->parent->first_child = dead->next_sibling;
        } else {
            scope_t* sib = dead->parent->first_child;
            while(sib->next_sibling && sib->next_sibling != dead){
                sib = sib->next_sibling;
            }
            if(sib->next_sibling == dead){
                sib->next_sibling = dead->next_sibling;
            }
        }
    }
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

    symbol_t* sym = arena_alloc(st->arena, sizeof(symbol_t), alignof(symbol_t));
    if(!sym) return NULL;

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

    hm_insert(scope->symbols, sym->name, sym);
    scope->count += 1;

    return sym;
}

static symbol_t* lookup_in_scope(scope_t* scope, const char* name)
{
    if(!scope || !name) return NULL;

    return hm_lookup(scope->symbols, name);
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

    scope_t* child = scope->first_child;
    while(child){
        scope_t* next_child = child->next_sibling;
        free_scope(child);
        child = next_child;
    }

    if(scope->symbols) {
        free_hashmap(scope->symbols);
        scope->symbols = NULL;
    }
    
    scope->parent = NULL;
    scope->first_child = NULL;
    scope->next_sibling = NULL;
    scope->owner = NULL;
}

void free_symbol_table(symbol_table_t* st)
{
    if(!st) return;

    if(st->global && st->global->symbols) {
        free_hashmap(st->global->symbols);
        st->global->symbols = NULL;
    }
    
    st->global = NULL;
    st->current = NULL;
}
