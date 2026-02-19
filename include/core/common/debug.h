#pragma once
#ifdef DEBUG

#include <stdio.h>
#include <stdbool.h>

#include "core/hashmap.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/semantic.h"

//
// LEXER
//

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

static inline void print_token(token_t token)
{
    const char* str_type = token_to_str(token);
    const char* literal = token.literal ? token.literal : "(null)";
    printf("\033[1m%-12s\033[0m%-10s  \033[0m%-5d%d\033[0m\n", str_type, literal, token.category, token.type);
}

//
// PARSER
//
static inline void print_node(node_t* node, int indent);

static inline void print_indent(int indent, const char* prefix)
{
    for(int i = 0; i < indent; i++){
        printf("  ");
    }
    if(prefix) printf("%s", prefix);
}

static inline void print_node_type(const char* type, const char* name, const char* color, bool bold)
{
    printf("%s%s%s%s%s", color, bold ? "\033[1m" : "", type, bold ? "\033[0m" : "", "\033[0m");
    if(name) printf(" %s\033[0m", name);
    printf("\n");
}

static inline void print_node(node_t* node, int indent)
{
    if(!node) return;

    print_indent(indent, NULL);

    switch(node->kind){
        case NODE_EXPR:
        case NODE_ASSIGN:
            printf("\033[90mEXPRESSION/ASSIGN\033[0m (unhandled display)\033[0m\n");
            break;
        case NODE_LITERAL:
            if(node->lit && node->lit->value.data){
                printf("\033[1mLITERAL\033[0m ");
                printf("\"%s\"\033[0m \033[90m[type:%d]\033[0m\n", node->lit->value.data, node->lit->type);
            }
            else {
                printf("\033[1mLITERAL\033[0m (null)\033[0m\n");
            }
            break;
        case NODE_REF:
            if(node->var_ref && node->var_ref->name.data){
                printf("\033[1mREFERENCE\033[0m ");
                printf("%s\033[0m\n", node->var_ref->name.data);
            }
            else {
                printf("\033[1mREFERENCE\033[0m (null)\033[0m\n");
            }
            break;
        case NODE_BINOP:
            if(node->binop && node->binop->lit){
                printf("\033[1mBINARY_OP\033[0m ");
                printf("%s\033[0m\n", node->binop->lit);
            }
            else {
                printf("\033[1mBINARY_OP\033[0m (null)\033[0m\n");
            }
            if(node->binop->left) print_node(node->binop->left, indent + 1);
            if(node->binop->right) print_node(node->binop->right, indent + 1);
            break;
        case NODE_VAR:
            if(node->var_decl && node->var_decl->name.data){
                printf("\033[1mVARIABLE\033[0m ");
                printf("%s\033[0m \033[90m[modif:%d, type:%d]\033[0m\n",
                       node->var_decl->name.data, node->var_decl->modif, node->var_decl->dtype);
            }
            else {
                printf("\033[1mVARIABLE\033[0m (null)\033[0m\n");
            }
            if(node->var_decl->value) print_node(node->var_decl->value, indent + 1);
            break;
        case NODE_BLOCK:
            printf("\033[1mBLOCK\033[0m \033[90m(%zu statements)\033[0m\n",
                   node->block ? node->block->statement.count : 0);
            if(node->block && node->block->statement.elems){
                for(size_t i = 0; i < node->block->statement.count; i++){
                    if(node->block->statement.elems[i]){
                        print_node(node->block->statement.elems[i], indent + 1);
                    }
                }
            }
            break;
        case NODE_UNARYOP:
            printf("\033[1mUNARY_OP\033[0m ");
            printf("%s\033[0m \033[90m[postfix:%s]\033[0m\n",
                   node->unaryop->lit ? node->unaryop->lit : "(null)",
                   node->unaryop->is_postfix ? "true" : "false");
            print_node(node->unaryop->right, indent + 1);
            break;
        case NODE_CALL:
            if(node->func_call && node->func_call->name.data){
                printf("\033[1mCALL\033[0m ");
                printf("%s\033[0m \033[90m(%zu args)\033[0m\n",
                       node->func_call->name.data, node->func_call->args.count);
            }
            else {
                printf("\033[1mCALL\033[0m (null)\033[0m\n");
            }
            if(node->func_call && node->func_call->args.elems){
                for(size_t i = 0; i < node->func_call->args.count; i++){
                    if(node->func_call->args.elems[i]){
                        print_node(node->func_call->args.elems[i], indent + 1);
                    }
                }
            }
            break;
        case NODE_MEMBER:
            if(node->member_decl && node->member_decl->name.data){
                printf("\033[1mMEMBER\033[0m ");
                printf("%s\033[0m\n", node->member_decl->name.data);
            }
            else {
                printf("\033[1mMEMBER\033[0m (null)\033[0m\n");
            }
            if(node->member_decl && node->member_decl->value){
                print_node(node->member_decl->value, indent + 1);
            }
            break;
        case NODE_RETURN:
            printf("\033[1mRETURN\033[0m\n");
            if(node->return_stmt && node->return_stmt->body){
                print_node(node->return_stmt->body, indent + 1);
            }
            break;
        case NODE_BREAK:
            printf("\033[1mBREAK\033[0m\n");
            break;
        case NODE_CONTINUE:
            printf("\033[1mCONTINUE\033[0m\n");
            break;
        case NODE_ARRAY:
            printf("\033[1mARRAY\033[0m (%zu elements)\033[0m\n",
                   node->array_decl ? node->array_decl->count : 0);
            if(node->array_decl && node->array_decl->elements){
                for(size_t i = 0; i < node->array_decl->count; i++){
                    if(node->array_decl->elements[i]){
                        print_node(node->array_decl->elements[i], indent + 1);
                    }
                }
            }
            break;
        case NODE_IF:
            printf("\033[1mIF_STATEMENT\033[0m\n");
            if(node->if_stmt){
                if(node->if_stmt->condition){
                    print_indent(indent + 1, "\033[90mCONDITION:\033[0m\n");
                    print_node(node->if_stmt->condition, indent + 2);
                }
                if(node->if_stmt->then_block){
                    print_indent(indent + 1, "\033[90mTHEN:\033[0m\n");
                    print_node(node->if_stmt->then_block, indent + 2);
                }
                if(node->if_stmt->elif_blocks){
                    print_indent(indent + 1, "\033[90mELSEIF:\033[0m\n");
                    print_node(node->if_stmt->elif_blocks, indent + 2);
                }
                if(node->if_stmt->else_block){
                    print_indent(indent + 1, "\033[90mELSE:\033[0m\n");
                    print_node(node->if_stmt->else_block, indent + 2);
                }
            }
            break;
        case NODE_WHILE:
            printf("\033[1mWHILE_LOOP\033[0m\n");
            if(node->while_stmt){
                if(node->while_stmt->condition){
                    print_indent(indent + 1, "\033[90mCONDITION:\033[0m\n");
                    print_node(node->while_stmt->condition, indent + 2);
                }
                if(node->while_stmt->body){
                    print_indent(indent + 1, "\033[90mBODY:\033[0m\n");
                    print_node(node->while_stmt->body, indent + 2);
                }
            }
            break;
        case NODE_FOR:
            printf("\033[1mFOR_LOOP\033[0m\n");
            if(node->for_stmt){
                if(node->for_stmt->init){
                    print_indent(indent + 1, "\033[90mINIT:\033[0m\n");
                    print_node(node->for_stmt->init, indent + 2);
                }
                if(node->for_stmt->condition){
                    print_indent(indent + 1, "\033[90mCONDITION:\033[0m\n");
                    print_node(node->for_stmt->condition, indent + 2);
                }
                if(node->for_stmt->update){
                    print_indent(indent + 1, "\033[90mUPDATE:\033[0m\n");
                    print_node(node->for_stmt->update, indent + 2);
                }
                if(node->for_stmt->body){
                    print_indent(indent + 1, "\033[90mBODY:\033[0m\n");
                    print_node(node->for_stmt->body, indent + 2);
                }
            }
            break;
        case NODE_PARAM:
            if(node->param_decl && node->param_decl->name.data){
                printf("\033[1mPARAMETER\033[0m ");
                printf("%s\033[0m \033[90m[variadic:%s, type:%d]\033[0m\n",
                       node->param_decl->name.data,
                       node->param_decl->is_variadic ? "true" : "false",
                       node->param_decl->dtype);
            }
            else {
                printf("\033[1mPARAMETER\033[0m (null)\033[0m\n");
            }
            break;
        case NODE_FUNC:
            if(node->func_decl && node->func_decl->name.data){
                printf("\033[1mFUNCTION\033[0m ");
                printf("%s\033[0m \033[90m(%zu params, return_type:%d)\033[0m\n",
                       node->func_decl->name.data,
                       node->func_decl->param_decl.count,
                       node->func_decl->return_type);
            }
            else {
                printf("\033[1mFUNCTION\033[0m (null)\033[0m\n");
            }
            if(node->func_decl && node->func_decl->param_decl.elems){
                print_indent(indent + 1, "\033[90mPARAMETERS:\033[0m\n");
                for(size_t i = 0; i < node->func_decl->param_decl.count; i++){
                    if(node->func_decl->param_decl.elems[i]){
                        print_node(node->func_decl->param_decl.elems[i], indent + 2);
                    }
                }
            }
            if(node->func_decl && node->func_decl->body){
                print_indent(indent + 1, "\033[90mBODY:\033[0m\n");
                print_node(node->func_decl->body, indent + 2);
            }
            break;
        case NODE_STRUCT:
            if(node->struct_decl && node->struct_decl->name.data){
                printf("\033[1mSTRUCT\033[0m ");
                printf("%s\033[0m \033[90m(%zu members)\033[0m\n",
                       node->struct_decl->name.data,
                       node->struct_decl->member.count);
            }
            else {
                printf("\033[1mSTRUCT\033[0m (anonymous)\033[0m\n");
            }
            if(node->struct_decl && node->struct_decl->member.elems){
                for(size_t i = 0; i < node->struct_decl->member.count; i++){
                    if(node->struct_decl->member.elems[i]){
                        print_node(node->struct_decl->member.elems[i], indent + 1);
                    }
                }
            }
            break;
        case NODE_ENUM:
            if(node->enum_decl && node->enum_decl->name.data){
                printf("\033[1mENUM\033[0m ");
                printf("%s\033[0m \033[90m(%zu members)\033[0m\n",
                       node->enum_decl->name.data,
                       node->enum_decl->member.count);
            }
            else {
                printf("\033[1mENUM\033[0m (anonymous)\033[0m\n");
            }
            if(node->enum_decl && node->enum_decl->member.elems){
                for(size_t i = 0; i < node->enum_decl->member.count; i++){
                    if(node->enum_decl->member.elems[i] &&
                       node->enum_decl->member.elems[i]->lit &&
                       node->enum_decl->member.elems[i]->lit->value.data){
                        print_indent(indent + 1, "");
                        printf("%s\033[0m\n", node->enum_decl->member.elems[i]->lit->value.data);
                    }
                }
            }
            break;
        case NODE_MATCH:
            printf("\033[1mMATCH\033[0m\n");
            if(node->match_stmt){
                if(node->match_stmt->target){
                    print_indent(indent + 1, "\033[90mTARGET:\033[0m\n");
                    print_node(node->match_stmt->target, indent + 2);
                }
                print_indent(indent + 1, "\033[90mCASES (%zu):\033[0m\n");
                for(size_t i = 0; i < node->match_stmt->block.count; i++){
                    if(node->match_stmt->block.elems[i]){
                        print_node(node->match_stmt->block.elems[i], indent + 2);
                    }
                }
            }
            break;
        case NODE_CASE:
            printf("\033[1mCASE\033[0m\n");
            if(node->case_stmt){
                if(node->case_stmt->condition){
                    print_indent(indent + 1, "\033[90mPATTERN:\033[0m\n");
                    print_node(node->case_stmt->condition, indent + 2);
                }
                if(node->case_stmt->body){
                    print_indent(indent + 1, "\033[90mBODY:\033[0m\n");
                    print_node(node->case_stmt->body, indent + 2);
                }
            }
            break;
        case NODE_TRAIT:
            if(node->trait_decl && node->trait_decl->name.data){
                printf("\033[1mTRAIT\033[0m ");
                printf("%s\033[0m\n", node->trait_decl->name.data);
            }
            else {
                printf("\033[1mTRAIT\033[0m (null)\033[0m\n");
            }
            if(node->trait_decl && node->trait_decl->body){
                print_node(node->trait_decl->body, indent + 1);
            }
            break;
        case NODE_IMPL:
            if(node->impl_decl && node->impl_decl->trait_name.data){
                printf("\033[1mIMPL\033[0m ");
                printf("%s for %s\033[0m\n",
                       node->impl_decl->trait_name.data,
                       node->impl_decl->struct_name.data);
            }
            else {
                printf("\033[1mIMPL\033[0m (null)\033[0m\n");
            }
            if(node->impl_decl && node->impl_decl->body){
                print_node(node->impl_decl->body, indent + 1);
            }
            break;
        case NODE_TRYCATCH:
            printf("\033[1mTRY-CATCH\033[0m\n");
            if(node->trycatch_stmt){
                if(node->trycatch_stmt->try_block){
                    print_indent(indent + 1, "\033[90mTRY:\033[0m\n");
                    print_node(node->trycatch_stmt->try_block, indent + 2);
                }
                if(node->trycatch_stmt->catch_block){
                    print_indent(indent + 1, "\033[90mCATCH:\033[0m\n");
                    print_node(node->trycatch_stmt->catch_block, indent + 2);
                }
                if(node->trycatch_stmt->finally_block){
                    print_indent(indent + 1, "\033[90mFINALLY:\033[0m\n");
                    print_node(node->trycatch_stmt->finally_block, indent + 2);
                }
            }
            break;
        case NODE_TYPE:
            if(node->type_decl && node->type_decl->name.data){
                printf("\033[1mTYPE_ALIAS\033[0m ");
                printf("%s\033[0m\n", node->type_decl->name.data);
            }
            else {
                printf("\033[1mTYPE_ALIAS\033[0m (null)\033[0m\n");
            }
            if(node->type_decl && node->type_decl->body){
                print_node(node->type_decl->body, indent + 1);
            }
            break;
        case NODE_IMPORT:
            printf("\033[1mIMPORT\033[0m (%zu modules)\033[0m\n",
                   node->import_decl ? node->import_decl->count : 0);
            if(node->import_decl){
                for(size_t i = 0; i < node->import_decl->count; ++i){
                    print_indent(indent + 1, "");
                    printf("%s\033[0m\n", node->import_decl->modules[i].data);
                }
            }
            break;
        case NODE_MODULE:
            if(node->module_decl && node->module_decl->name.data){
                printf("\033[1mMODULE\033[0m ");
                printf("%s\033[0m\n", node->module_decl->name.data);
            }
            else {
                printf("\033[1mMODULE\033[0m (null)\033[0m\n");
            }
            if(node->module_decl && node->module_decl->body){
                print_node(node->module_decl->body, indent + 1);
            }
            break;
        case NODE_NAMEOF:
            printf("\033[1mNAMEOF\033[0m ");
            if(node->special_stmt && node->special_stmt->content.data){
                printf("\"%s\"\033[0m\n", node->special_stmt->content.data);
            }
            else {
                printf("(null)\033[0m\n");
            }
            break;
        case NODE_TYPEOF:
            printf("\033[1mTYPEOF\033[0m ");
            if(node->special_stmt && node->special_stmt->content.data){
                printf("\"%s\"\033[0m\n", node->special_stmt->content.data);
            }
            else {
                printf("(null)\033[0m\n");
            }
            break;
    }
}

//
// SEMANTIC
//

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
        printf("NONE"); return;
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
            symbol_t* sym = entry->value;
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
        printf("Symbol table: (null)\n"); return;
    }

    printf("Total scopes: %zu\n", st->scope_count);
    printf("Current scope depth: %d\n", st->current ? st->current->depth : -1);
    printf("\n");

    if(st->global) print_scope(st->global, 0);
}

static inline void print_current_scope(symbol_table_t* st)
{
    if(!st || !st->current){
        printf("Current scope: (null)\n"); return;
    }

    print_scope(st->current, 0);
}

static inline void print_symbol_lookup(symbol_table_t* st, const char* name)
{
    if(!st || !name){
        printf("Symbol lookup: invalid parameters\n"); return;
    }

    symbol_t* sym = lookup_symbol(st, name);
    if(sym){
        print_symbol(sym, 0);
    }
    else {
        printf("\033[31mSymbol '%s' not found\033[0m\n", name);
    }
}

#define print_token(t) print_token(t)
#define print_token_list(tokens, count) print_token_list(tokens, count)
#define print_ast(n, i) print_node(n, i)
#define print_symbol_table(st) print_symbol_table(st)
#define print_current_scope(st) print_current_scope(st)
#define print_symbol_lookup(st, name) print_symbol_lookup(st, name)
#define print_symbol(sym, indent) print_symbol(sym, indent)

#endif
