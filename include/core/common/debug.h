#pragma once
#ifdef DEBUG

#include <stdio.h>
#include <stdbool.h>

#include "core/hashmap.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/semantic.h"
#include "compiler/middle/ir.h"

static inline const char* token_to_str(const token_t token)
{
    switch(token.category){
        case CAT_SERVICE:
            switch(token.type){
                case SERV_ILLEGAL: return "ILLEGAL";
                default: return "SERVICE";
            }
            break;
        case CAT_LITERAL:
            switch(token.type){
                case LIT_IDENT:
                    return "IDENT";
                case LIT_STRING:
                    return "STRING";
                case LIT_CHAR:
                    return "CHAR";
                case LIT_TRUE:
                case LIT_FALSE:
                    return "BOOLEAN";
                case LIT_NUMBER:
                case LIT_FLOAT:
                case LIT_HEX:
                case LIT_BIN:
                case LIT_OCT:
                case LIT_INFINITY:
                    return "NUMBER";
                default:
                    return "LITERAL";
            }
            break;
        case CAT_DATATYPE:  return "DATATYPE";
        case CAT_KEYWORD:   return "KEYWORD";
        case CAT_DELIMITER: return "DELIMITER";
        case CAT_OPERATOR:  return "OPERATOR";
        case CAT_MODIFIER:  return "MODIFIER";
        case CAT_PAREN:     return "PAREN";
        default: return "UNKNOWN";
    }
    return NULL;
}

static inline void print_node(node_t* node, int indent)
{
    if(!node) return;

    for(int i = 0; i < indent; i++){
        printf("  ");
    }

    switch(node->kind){
        case NODE_EXPR:
        case NODE_ASSIGN:
            printf("Expression / Assignment (Unhandled display for now)");
            break;
        case NODE_LITERAL:
            if(node->lit && node->lit->value.data)
                printf("Literal: %s\n", node->lit->value.data);
            else
                printf("Literal: (null)\n");
            break;
        case NODE_REF:
            if(node->var_ref && node->var_ref->name.data)
                printf("Ref: %s\n", node->var_ref->name.data);
            else
                printf("Ref: (null)\n");
            break;
        case NODE_BINOP:
            if(node->binop && node->binop->lit)
                printf("BinOp: %s\n", node->binop->lit);
            else
                printf("BinOp: (null)\n");
            if(node->binop->left) print_node(node->binop->left, indent + 1);
            if(node->binop->right) print_node(node->binop->right, indent + 1);
            break;
        case NODE_VAR:
            if(node->var_decl && node->var_decl->name.data)
                printf("Var: %s\n", node->var_decl->name.data);
            else
                printf("Var: (null)\n");
            if(node->var_decl->value) print_node(node->var_decl->value, indent + 1);
            break;
        case NODE_BLOCK:
            printf("Block:\n");
            if(node->block && node->block->statement.elems){
                for(size_t i = 0; i < node->block->statement.count; i++){
                    if(node->block->statement.elems[i])
                        print_node(node->block->statement.elems[i], indent + 1);
                }
            }
            break;
        case NODE_UNARYOP:
            printf("UnaryOp: %s\n", node->unaryop->lit ? node->unaryop->lit : "(null)");
            print_node(node->unaryop->right, indent + 1);
            break;
        case NODE_CALL:
            if(node->func_call && node->func_call->name.data)
                printf("Call: %s\n", node->func_call->name.data);
            else
                printf("Call: (null)\n");
            if(node->func_call && node->func_call->args.elems){
                for(size_t i = 0; i < node->func_call->args.count; i++){
                    if(node->func_call->args.elems[i])
                        print_node(node->func_call->args.elems[i], indent + 1);
                }
            }
            break;
        case NODE_MEMBER:
            if(node->member_decl && node->member_decl->name.data)
                printf("Enum Member: %s\n", node->member_decl->name.data);
            else
                printf("Enum Member: (null)\n");
            break;
        case NODE_RETURN:
            printf("Return:\n");
            if(node->return_stmt && node->return_stmt->body)
                print_node(node->return_stmt->body, indent + 1);
            break;
        case NODE_BREAK:
            printf("Break:\n");
            break;
        case NODE_CONTINUE:
            printf("Continue:\n");
            break;
        case NODE_ARRAY:
            printf("Array:\n");
            if(node->array_decl && node->array_decl->elements){
                for(size_t i = 0; i < node->array_decl->count; i++){
                    if(node->array_decl->elements[i])
                        print_node(node->array_decl->elements[i], indent + 1);
                }
            }
            break;
        case NODE_IF:
            printf("If Statement:\n");
            if(node->if_stmt){
                if(node->if_stmt->condition) print_node(node->if_stmt->condition, indent + 1);
                if(node->if_stmt->then_block) print_node(node->if_stmt->then_block, indent + 1);
                if(node->if_stmt->elif_blocks) print_node(node->if_stmt->elif_blocks, indent + 1);
                if(node->if_stmt->else_block) print_node(node->if_stmt->else_block, indent + 1);
            }
            break;
        case NODE_WHILE:
            printf("While Loop:\n");
            if(node->while_stmt){
                if(node->while_stmt->condition) print_node(node->while_stmt->condition, indent + 1);
                if(node->while_stmt->body) print_node(node->while_stmt->body, indent + 1);
            }
            break;
        case NODE_FOR:
            printf("For Loop:\n");
            if(node->for_stmt){
                if(node->for_stmt->init) print_node(node->for_stmt->init, indent + 1);
                if(node->for_stmt->condition) print_node(node->for_stmt->condition, indent + 1);
                if(node->for_stmt->update) print_node(node->for_stmt->update, indent + 1);
                if(node->for_stmt->body) print_node(node->for_stmt->body, indent + 1);
            }
            break;
        case NODE_PARAM:
            if(node->param_decl && node->param_decl->name.data)
                printf("Function Parameter: %s\n", node->param_decl->name.data);
            else
                printf("Function Parameter: (null)\n");
            break;
        case NODE_FUNC:
            if(node->func_decl && node->func_decl->name.data)
                printf("Function: %s\n", node->func_decl->name.data);
            else
                printf("Function: (null)\n");
            if(node->func_decl && node->func_decl->param_decl.elems){
                for(size_t i = 0; i < node->func_decl->param_decl.count; i++){
                    if(node->func_decl->param_decl.elems[i])
                        print_node(node->func_decl->param_decl.elems[i], indent + 1);
                }
            }
            if(node->func_decl && node->func_decl->body) print_node(node->func_decl->body, indent + 1);
            break;
        case NODE_STRUCT:
            if(node->struct_decl && node->struct_decl->name.data)
                printf("Struct: %s\n", node->struct_decl->name.data);
            else
                printf("Struct: (anonymous)\n");
            if(node->struct_decl && node->struct_decl->member.elems){
                for(size_t i = 0; i < node->struct_decl->member.count; i++){
                    if(node->struct_decl->member.elems[i])
                        print_node(node->struct_decl->member.elems[i], indent + 1);
                }
            }
            break;
        case NODE_ENUM:
            if(node->enum_decl && node->enum_decl->name.data)
                printf("Enum: %s\n", node->enum_decl->name.data);
            else
                printf("Enum: (anonymous)\n");
            if(node->enum_decl && node->enum_decl->member.elems){
                for(size_t i = 0; i < node->enum_decl->member.count; i++){
                    if(node->enum_decl->member.elems[i] && node->enum_decl->member.elems[i]->lit && node->enum_decl->member.elems[i]->lit->value.data)
                        printf("  %s\n", node->enum_decl->member.elems[i]->lit->value.data);
                }
            }
            break;
        case NODE_MATCH:
            printf("Match Statement:\n");
            print_node(node->match_stmt->target, indent + 1);
            for(size_t i = 0; i < node->match_stmt->block.count; i++){
                print_node(node->match_stmt->block.elems[i], indent + 1);
            }
            break;
        case NODE_CASE:
            printf("Case:\n");
            print_node(node->case_stmt->condition, indent + 1);
            print_node(node->case_stmt->body, indent + 1);
            break;
        case NODE_TRAIT:
            printf("Trait: %s\n", node->trait_decl->name.data);
            print_node(node->trait_decl->body, indent + 1);
            break;
        case NODE_IMPL:
            printf("Impl: %s\n", node->impl_decl->trait_name.data);
            print_node(node->trait_decl->body, indent + 1);
            break;
        case NODE_TRYCATCH:
            printf("Try-Catch:\n");
            print_node(node->trycatch_stmt->try_block, indent + 1);
            if(node->trycatch_stmt->catch_block){
                print_node(node->trycatch_stmt->catch_block, indent + 1);
            }
            if(node->trycatch_stmt->finally_block){
                print_node(node->trycatch_stmt->finally_block, indent + 1);
            }
            break;
        case NODE_TYPE:
            printf("Type: %s\n", node->type_decl->name.data);
            print_node(node->type_decl->body, indent + 1);
            break;
        case NODE_IMPORT:
            printf("Import:\n");
            for(size_t i = 0; i < node->import_decl->count; ++i){
                printf("Module: %s\n", node->import_decl->modules[i].data);
            }
            break;
        case NODE_MODULE:
            printf("Module: %s\n", node->module_decl->name.data);
            print_node(node->module_decl->body, indent + 1);
            break;
        case NODE_NAMEOF:
        case NODE_TYPEOF:
            printf("Reflection:\n");
            printf("\tContent: %s\n", node->special_stmt->content.data);
            break;
    }
}

static inline const char* symbol_kind_to_str(enum symbol_kind kind)
{
    switch(kind){
        case SYMBOL_VAR: return "VAR";
        case SYMBOL_CONST: return "CONST";
        case SYMBOL_FUNC: return "FUNC";
        case SYMBOL_PARAM: return "PARAM";
        case SYMBOL_STRUCT: return "STRUCT";
        case SYMBOL_ENUM: return "ENUM";
        case SYMBOL_ENUM_VARIANT: return "ENUM_VARIANT";
        case SYMBOL_TYPE_ALIAS: return "TYPE_ALIAS";
        case SYMBOL_BUILTIN_TYPE: return "BUILTIN_TYPE";
        case SYMBOL_MODULE: return "MODULE";
        case SYMBOL_GENERIC: return "GENERIC";
        default: return "UNKNOWN";
    }
}

static inline const char* scope_kind_to_str(enum scope_kind kind)
{
    switch(kind){
        case SCOPE_GLOBAL: return "GLOBAL";
        case SCOPE_MODULE: return "MODULE";
        case SCOPE_FUNCTION: return "FUNCTION";
        case SCOPE_BLOCK: return "BLOCK";
        case SCOPE_STRUCT: return "STRUCT";
        case SCOPE_ENUM: return "ENUM";
        case SCOPE_SWITCH: return "SWITCH";
        case SCOPE_LOOP: return "LOOP";
        default: return "UNKNOWN";
    }
}

static inline const char* type_kind_to_str(enum type_kind kind)
{
    switch(kind){
        case TYPE_VOID: return "VOID";
        case TYPE_ANY: return "ANY";
        case TYPE_BOOL: return "BOOL";
        case TYPE_INT: return "INT";
        case TYPE_UINT: return "UINT";
        case TYPE_SHORT: return "SHORT";
        case TYPE_USHORT: return "USHORT";
        case TYPE_LONG: return "LONG";
        case TYPE_ULONG: return "ULONG";
        case TYPE_FLOAT: return "FLOAT";
        case TYPE_DECIMAL: return "DECIMAL";
        case TYPE_STR: return "STR";
        case TYPE_CHAR: return "CHAR";
        case TYPE_ARRAY: return "ARRAY";
        case TYPE_FUNC: return "FUNC";
        case TYPE_STRUCT: return "STRUCT";
        case TYPE_ENUM: return "ENUM";
        default: return "UNKNOWN";
    }
}

static inline void print_symbol_flags(enum symbol_flags flags)
{
    if(flags == SYM_FLAG_NONE){
        printf("NONE");
        return;
    }

    bool first = true;
    if(flags & SYM_FLAG_USED){ printf("%sUSED", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_ASSIGNED){ printf("%sASSIGNED", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_GLOBAL){ printf("%sGLOBAL", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_EXTERN){ printf("%sEXTERN", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_STATIC){ printf("%sSTATIC", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_MUTABLE){ printf("%sMUTABLE", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_PRIVATE){ printf("%sPRIVATE", first ? "" : "|"); first = false; }
    if(flags & SYM_FLAG_PUBLIC){ printf("%sPUBLIC", first ? "" : "|"); first = false; }
}

static inline void print_symbol(symbol_t* sym, int indent)
{
    if(!sym) return;

    for(int i = 0; i < indent; i++) printf("  ");

    printf("\033[33m%s\033[0m \033[1m%s\033[0m", symbol_kind_to_str(sym->kind), sym->name);

    printf(" [flags: ");
    print_symbol_flags(sym->flags);
    printf("] [loc: %ld:%ld]", sym->loc.line, sym->loc.column);

    if(sym->type) printf(" [type: %s]", type_kind_to_str(sym->type->kind));
    if(sym->scope) printf(" [scope: %s depth:%d]", scope_kind_to_str(sym->scope->kind), sym->scope->depth);

    printf("\n");

    if(sym->shadowed_symbol){
        for(int i = 0; i < indent + 1; i++) printf("  ");
        printf("  \033[31mshadows:\033[0m %s\n", sym->shadowed_symbol->name);
    }

    if(sym->overload_next){
        for(int i = 0; i < indent + 1; i++) printf("  ");
        printf("  \033[36moverload:\033[0m %s\n", sym->overload_next->name);
    }
}

static inline void print_scope_symbols(scope_t* scope, int indent)
{
    if(!scope || !scope->symbols) return;

    for(int i = 0; i < indent; i++) printf("  ");
    printf("\033[32mSymbols (%zu):\033[0m\n", scope->count);

    hashmap_t* entry = scope->symbols;
    for(size_t i = 0; i < scope->symbols->count; i++){
        if(entry->key.data && entry->value){
            symbol_t* sym = (symbol_t*)entry->value;
            print_symbol(sym, indent + 1);
        }
        entry = scope->symbols->next;
    }
}

static inline void print_scope(scope_t* scope, int indent)
{
    if(!scope) return;

    for(int i = 0; i < indent; i++) printf("  ");
    printf("\033[34mScope\033[0m \033[1m%s\033[0m\n", scope_kind_to_str(scope->kind));

    print_scope_symbols(scope, indent + 1);

    if(scope->first_child){
        for(int i = 0; i < indent; i++) printf("  ");
        printf("\033[35mChild scopes:\033[0m\n");

        scope_t* child = scope->first_child;
        while(child){
            print_scope(child, indent + 1);
            child = child->next_sibling;
        }
    }
}

static inline void print_symbol_table(symbol_table_t* st)
{
    if(!st){
        printf("Symbol table: (null)\n");
        return;
    }

    printf("Total scopes: %zu\n", st->scope_count);
    printf("Current scope depth: %d\n", st->current ? st->current->depth : -1);
    printf("\n");

    if(st->global) print_scope(st->global, 0);
}

static inline void print_current_scope(symbol_table_t* st)
{
    if(!st || !st->current){
        printf("Current scope: (null)\n");
        return;
    }

    print_scope(st->current, 0);
}

static inline void print_symbol_lookup(symbol_table_t* st, const char* name)
{
    if(!st || !name){
        printf("Symbol lookup: invalid parameters\n");
        return;
    }

    symbol_t* sym = lookup_symbol(st, name);
    if(sym){
        print_symbol(sym, 0);
    }
    else {
        printf("\033[31mSymbol '%s' not found\033[0m\n", name);
    }
}


#define print_token(t) do{ printf("\033[34m%s\033[0m(\033[1m%s\033[0m)\n", token_to_str(t), t.literal); } while(0)
#define print_ast(n, i) print_node(n, i)
#define print_symbol_table_debug(st) print_symbol_table(st)
#define print_current_scope_debug(st) print_current_scope(st)
#define print_symbol_lookup_debug(st, name) print_symbol_lookup(st, name)
#define print_symbol_debug(sym, indent) print_symbol(sym, indent)

#endif
