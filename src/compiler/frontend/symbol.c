#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "compiler/frontend/symbol.h"
#include "compiler/core/diagnostic.h"
#include "common/utils.h"

#define INITIAL_SCOPE_CAPACITY 16
#define SYMBOL_TABLE_SIZE 64

scope_t* new_scope(int kind, astnode_t* owner);
void free_scope(scope_t* scope);

scope_t* new_scope(int kind, astnode_t* owner)
{
    scope_t* scope = (scope_t*)malloc(sizeof(scope_t));
    if(!scope) return NULL;
    
    scope->parent = NULL;
    scope->kind = kind;
    scope->owner = owner;
    scope->count = 0;
    scope->capacity = SYMBOL_TABLE_SIZE;
    
    scope->symbols = (symbol_t**)calloc(SYMBOL_TABLE_SIZE, sizeof(symbol_t*));
    if(!scope->symbols){
        free(scope);
        return NULL;
    }
    
    return scope;
}

symbol_table_t* new_symbol_table(void)
{
    symbol_table_t* st = (symbol_table_t*)malloc(sizeof(symbol_table_t));
    if(!st) return NULL;
    
    st->global = new_scope(SCOPE_GLOBAL, NULL);
    if(!st->global){
        free(st);
        return NULL;
    }
    
    st->current = st->global;
    st->scope_capacity = INITIAL_SCOPE_CAPACITY;
    st->scope_count = 1;
    
    st->scopes = (scope_t**)malloc(st->scope_capacity * sizeof(scope_t*));
    if(!st->scopes){
        free_scope(st->global);
        free(st);
        return NULL;
    }
    
    st->scopes[0] = st->global;
    return st;
}

unsigned int hash_string(const char* str)
{
    unsigned int hash = 5381;
    int c;
    while((c = *str++)){
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash % SYMBOL_TABLE_SIZE;
}

scope_t* push_scope(symbol_table_t* st, int scope_kind, astnode_t* owner)
{
    if(!st) return NULL;
    
    scope_t* scope = new_scope(scope_kind, owner);
    if(!scope) return NULL;
    
    scope->parent = st->current;
    st->current = scope;
    
    // add to scopes list
    if(st->scope_count >= st->scope_capacity){
        size_t new_cap = st->scope_capacity * 2;
        scope_t** scopes = (scope_t**)realloc(st->scopes, new_cap * sizeof(scope_t*));
        if(!scopes){
            free_scope(scope);
            return NULL;
        }
        st->scopes = scopes;
        st->scope_capacity = new_cap;
    }
    
    st->scopes[st->scope_count++] = scope;
    return scope;
}

void pop_scope(symbol_table_t* st)
{
    if(!st || !st->current || st->current == st->global) return;
    st->current = st->current->parent;
}

scope_t* current_scope(symbol_table_t* st)
{
    return st ? st->current : NULL;
}

symbol_t* define_symbol(symbol_table_t* st, const char* name, const enum symbol_kind kind, type_t* type, astnode_t* decl_node)
{
    if(!st || !name) return NULL;
    
    scope_t* scope = st->current;
    unsigned int hash = hash_string(name);
    
    // Check for redefinition in current scope
    symbol_t* existing = scope->symbols[hash];
    while(existing){
        if(strcmp(existing->name, name) == 0){
            return NULL;  // Symbol already exists
        }
        existing = existing->next;
    }
    
    // Create new symbol
    symbol_t* sym = (symbol_t*)malloc(sizeof(symbol_t));
    if(!sym) return NULL;
    
    sym->name = util_strdup(name);
    if(!sym->name){
        free(sym);
        return NULL;
    }
    
    sym->kind = kind;
    sym->type = type;
    sym->flags = SYM_FLAG_NONE;
    sym->decl_node = decl_node;
    sym->line = decl_node ? decl_node->line : 0;
    sym->column = 0;
    sym->scope = NULL;
    
    // Mark as global if in global scope
    if(scope == st->global){
        sym->flags |= SYM_FLAG_GLOBAL;
    }
    
    // Insert at head of hash chain
    sym->next = scope->symbols[hash];
    scope->symbols[hash] = sym;
    scope->count++;
    
    return sym;
}

symbol_t* lookup_in_scope(scope_t* scope, const char* name)
{
    if(!scope || !name) return NULL;
    
    unsigned int hash = hash_string(name);
    symbol_t* sym = scope->symbols[hash];
    
    while(sym){
        if(strcmp(sym->name, name) == 0){
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

symbol_t* lookup_symbol(symbol_table_t* st, const char* name)
{
    if(!st || !name) return NULL;
    
    // Search from current scope up to global
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
    
    for(size_t i = 0; i < scope->capacity; i++){
        symbol_t* sym = scope->symbols[i];
        while(sym){
            symbol_t* next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    
    free_ast(scope->owner);
    free(scope->symbols);
    free(scope);
    
    scope->parent = NULL;
    scope->symbols = NULL;
    scope = NULL;
}

void free_symbol_table(symbol_table_t* st)
{
    if(!st) return;
    
    for(size_t i = 0; i < st->scope_count; i++){
        free_scope(st->scopes[i]);
        st->scopes[i] = NULL;
    }
    
    free_scope(st->global);
    free_scope(st->current);
    free(st->scopes);
    
    st->global = NULL;
    st->current = NULL;
    st->scopes = NULL;
    
    free(st);
    st = NULL;
}
