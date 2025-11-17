#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "compiler/frontend/symbol.h"
#include "compiler/core/diagnostic.h"
#include "common/utils.h"

#define INITIAL_SCOPE_CAPACITY 16
#define SYMBOL_TABLE_SIZE 64

struct scope* new_scope(int kind, struct ast_node* owner);
void free_scope(struct scope* scope);

struct scope* new_scope(int kind, struct ast_node* owner)
{
    struct scope* scope = (struct scope*)malloc(sizeof(struct scope));
    if(!scope) return NULL;
    
    scope->parent = NULL;
    scope->kind = kind;
    scope->owner = owner;
    scope->count = 0;
    scope->capacity = SYMBOL_TABLE_SIZE;
    
    scope->symbols = (struct symbol**)calloc(SYMBOL_TABLE_SIZE, sizeof(struct symbol*));
    if(!scope->symbols){
        free(scope);
        return NULL;
    }
    
    return scope;
}

struct symbol_table* new_symbol_table(void)
{
    struct symbol_table* st = (struct symbol_table*)malloc(sizeof(struct symbol_table));
    if(!st) return NULL;
    
    st->global = new_scope(SCOPE_GLOBAL, NULL);
    if(!st->global){
        free(st);
        return NULL;
    }
    
    st->current = st->global;
    st->scope_capacity = INITIAL_SCOPE_CAPACITY;
    st->scope_count = 1;
    
    st->scopes = (struct scope**)malloc(st->scope_capacity * sizeof(struct scope*));
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

struct scope* push_scope(struct symbol_table* st, int scope_kind, struct ast_node* owner)
{
    if(!st) return NULL;
    
    struct scope* scope = new_scope(scope_kind, owner);
    if(!scope) return NULL;
    
    scope->parent = st->current;
    st->current = scope;
    
    // add to scopes list
    if(st->scope_count >= st->scope_capacity){
        size_t new_cap = st->scope_capacity * 2;
        struct scope** scopes = (struct scope**)realloc(st->scopes, new_cap * sizeof(struct scope*));
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

void pop_scope(struct symbol_table* st)
{
    if(!st || !st->current || st->current == st->global) return;
    st->current = st->current->parent;
}

struct scope* current_scope(struct symbol_table* st)
{
    return st ? st->current : NULL;
}

struct symbol* define_symbol(struct symbol_table* st, const char* name, const enum symbol_kind kind, struct type* type, struct ast_node* decl_node)
{
    if(!st || !name) return NULL;
    
    struct scope* scope = st->current;
    unsigned int hash = hash_string(name);
    
    // Check for redefinition in current scope
    struct symbol* existing = scope->symbols[hash];
    while(existing){
        if(strcmp(existing->name, name) == 0){
            return NULL;  // Symbol already exists
        }
        existing = existing->next;
    }
    
    // Create new symbol
    struct symbol* sym = (struct symbol*)malloc(sizeof(struct symbol));
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

struct symbol* lookup_in_scope(struct scope* scope, const char* name)
{
    if(!scope || !name) return NULL;
    
    unsigned int hash = hash_string(name);
    struct symbol* sym = scope->symbols[hash];
    
    while(sym){
        if(strcmp(sym->name, name) == 0){
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

struct symbol* lookup_symbol(struct symbol_table* st, const char* name)
{
    if(!st || !name) return NULL;
    
    // Search from current scope up to global
    struct scope* scope = st->current;
    while(scope){
        struct symbol* sym = lookup_in_scope(scope, name);
        if(sym) return sym;
        scope = scope->parent;
    }
    
    return NULL;
}

bool is_scope_symbol_exist(struct symbol_table* st, const char* name)
{
    if(!st || !name) return false;
    return lookup_in_scope(st->current, name) != NULL;
}

bool is_symbol_mutable(const struct symbol* sym)
{
    if(!sym) return false;
    return sym->kind != SYMBOL_CONST;
}

bool is_symbol_initialized(const struct symbol* sym)
{
    if(!sym) return false;
    return (sym->flags & SYM_FLAG_ASSIGNED) != 0;
}

void free_scope(struct scope* scope)
{
    if(!scope) return;
    
    for(size_t i = 0; i < scope->capacity; i++){
        struct symbol* sym = scope->symbols[i];
        while(sym){
            struct symbol* next = sym->next;
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

void free_symbol_table(struct symbol_table* st)
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
