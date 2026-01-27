#pragma once
#ifdef DEBUG

#include <stdio.h>
#include <stdbool.h>

#include "compiler/core/hash_table.h"
#include "compiler/frontend/tokenizer.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/types.h"
#include "compiler/frontend/symbol.h"
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
            printf("Literal: %s\n", node->lit->value.data);
            break;
        case NODE_REF:
            printf("Ref: %s\n", node->var_ref->name.data);
            break;
        case NODE_BINOP:
            printf("BinOp: %s\n", node->binop->lit);
            print_node(node->binop->left, indent + 1);
            print_node(node->binop->right, indent + 1);
            break;
        case NODE_VAR:
            printf("Var: %s\n", node->var_decl->name.data);
            print_node(node->var_decl->value, indent + 1);
            break;
        case NODE_BLOCK:
            printf("Block:\n");
            for(size_t i = 0; i < node->block->statement.count; i++){
                print_node(node->block->statement.elems[i], indent + 1);
            }
            break;
        case NODE_UNARYOP:
            printf("UnaryOp: %s\n", node->unaryop->lit);
            print_node(node->unaryop->right, indent + 1);
            break;
        case NODE_CALL:
            printf("Call: %s\n", node->call->name.data);
            for(size_t i = 0; i < node->call->args.count; i++){
                print_node(node->call->args.elems[i], indent + 1);
            }
            break;
        case NODE_ENUM_MEMBER:
            printf("Enum Member: %s\n", node->enum_member->name.data);
            break;
        case NODE_RETURN:
            printf("Return:\n");
            print_node(node->ret->body, indent + 1);
            break;
        case NODE_BREAK:
            printf("Break:\n");
            break;
        case NODE_CONTINUE:
            printf("Continue:\n");
            break;
        case NODE_ARRAY:
            printf("Array:\n");
            for(size_t i = 0; i < node->array_decl->count; i++){
                print_node(node->array_decl->elements[i], indent + 1);
            }
            break;
        case NODE_IF:
            printf("If Statement:\n");
            print_node(node->if_stmt->condition, indent + 1);
            print_node(node->if_stmt->then_block, indent + 1);
            if(node->if_stmt->elif_blocks){
                print_node(node->if_stmt->elif_blocks, indent + 1);
            }
            if(node->if_stmt->else_block){
                print_node(node->if_stmt->else_block, indent + 1);
            }
            break;
        case NODE_WHILE:
            printf("While Loop:\n");
            print_node(node->while_loop->condition, indent + 1);
            print_node(node->while_loop->body, indent + 1);
            break;
        case NODE_FOR:
            printf("For Loop:\n");
            print_node(node->for_loop->init, indent + 1);
            print_node(node->for_loop->condition, indent + 1);
            print_node(node->for_loop->update, indent + 1);
            print_node(node->for_loop->body, indent + 1);
            break;
        case NODE_VAR_PARAM:
            printf("Function Parameter: %s\n", node->func_param->name.data);
            break;
        case NODE_FUNC:
            printf("Function: %s\n", node->func_decl->name.data);
            for(size_t i = 0; i < node->func_decl->param.count; i++){
                print_node(node->func_decl->param.elems[i], indent + 1);
            }
            print_node(node->func_decl->body, indent + 1);
            break;
        case NODE_STRUCT:
            printf("Struct: %s\n", node->struct_decl->name.data ? node->struct_decl->name.data : "(anonymous)");
            for(size_t i = 0; i < node->struct_decl->member.count; i++){
                print_node(node->struct_decl->member.elems[i], indent + 1);
            }
            break;
        case NODE_ENUM:
            printf("Enum: %s\n", node->enum_decl->name.data ? node->enum_decl->name.data : "(anonymous)");
            for(size_t i = 0; i < node->enum_decl->member.count; i++){
                printf("  %s\n", node->enum_decl->member.elems[i]->lit->value.data);
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
            print_node(node->match_case->condition, indent + 1);
            print_node(node->match_case->body, indent + 1);
            break;
        case NODE_TRAIT:
            printf("Trait: %s\n", node->trait_decl->name.data);
            print_node(node->trait_decl->body, indent + 1);
            break;
        case NODE_IMPL:
            printf("Impl: %s\n", node->impl_stmt->trait_name.data);
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
            printf("\tContent: %s\n", node->spec_stmt->content.data);
            break;
    }
}

static inline const char* type_to_string(const type_t* type)
{
    if(!type) return "<null>";

    switch(type->kind){
        case TYPE_UNKNOWN: return "<unknown>";
        case TYPE_ERROR:   return "<error>";
        case TYPE_VOID:    return "void";
        case TYPE_BOOL:    return "bool";
        case TYPE_INT:     return "int";
        case TYPE_UINT:    return "uint";
        case TYPE_SHORT:   return "short";
        case TYPE_USHORT:  return "ushort";
        case TYPE_LONG:    return "long";
        case TYPE_ULONG:   return "ulong";
        case TYPE_FLOAT:   return "float";
        case TYPE_DECIMAL: return "decimal";
        case TYPE_STR:     return "str";
        case TYPE_CHAR:    return "char";
        case TYPE_ARRAY:   return "array";
        case TYPE_FUNC:    return "function";
        case TYPE_STRUCT:  return "struct";
        case TYPE_ENUM:    return "enum";
        default:           return "<invalid>";
    }
}

static inline const char* symbol_kind_name(enum symbol_kind kind)
{
    switch(kind){
        case SYMBOL_VAR:        return "variable";
        case SYMBOL_CONST:      return "constant";
        case SYMBOL_FUNC:       return "function";
        case SYMBOL_PARAM:      return "parameter";
        case SYMBOL_STRUCT:     return "struct";
        case SYMBOL_ENUM:       return "enum";
        case SYMBOL_UNION:      return "union";
        case SYMBOL_TYPE_ALIAS: return "type";
        case SYMBOL_BUILTIN_TYPE: return "builtin type";
        case SYMBOL_MODULE:     return "module";
        case SYMBOL_GENERIC:    return "generic";
        default:                return "unknown";
    }
}

static inline void print_scope(const scope_t* scope, int indent)
{
    if(!scope) return;

    const char* scope_name[] = {"global", "function", "block", "struct"};
    printf("%*sScope: %s (%zu symbols)\n", indent, "", scope_name[scope->kind], scope->count);

    for(size_t i = 0; i < scope->count; i++){
        symbol_t* sym = ht_lookup(scope->symbols, (void*)(uintptr_t)i);
        while(sym){
            printf("%*s  %s '%s': %s\n", indent, "", symbol_kind_name(sym->kind), sym->name, type_to_string(sym->type));
            // sym = sym->next;
        }
    }
}

static inline void print_symbol_table(const symbol_table_t* st)
{
    if(!st) return;

    printf("=== Symbol Table ===\n");
    // for(size_t i = 0; i < st->scope_count; i++) print_scope(st->scopes[i], i * 2);
}

#define OPCODE_TO_STR(oc)                          \
    _Generic((oc),                                 \
        enum op_code:                              \
            (oc) == OP_PUSH     ? "PUSH" :         \
            (oc) == OP_POP      ? "POP" :          \
            (oc) == OP_DUP      ? "DUP" :          \
            (oc) == OP_ADD      ? "ADD" :          \
            (oc) == OP_SUB      ? "SUB" :          \
            (oc) == OP_MUL      ? "MUL" :          \
            (oc) == OP_DIV      ? "DIV" :          \
            (oc) == OP_AND      ? "AND" :          \
            (oc) == OP_OR       ? "OR" :           \
            (oc) == OP_NOT      ? "NOT" :          \
            (oc) == OP_EQ       ? "EQ" :           \
            (oc) == OP_NEQ      ? "NEQ" :          \
            (oc) == OP_LT       ? "LT" :           \
            (oc) == OP_GT       ? "GT" :           \
            (oc) == OP_STORE    ? "STORE" :        \
            (oc) == OP_LOAD     ? "LOAD" :         \
            (oc) == OP_STORE_GLOB ? "STORE_GLOB" : \
            (oc) == OP_LOAD_GLOB  ? "LOAD_GLOB" :  \
            (oc) == OP_JUMP     ? "JUMP" :         \
            (oc) == OP_CALL     ? "CALL" :         \
            (oc) == OP_RETURN   ? "RETURN" :       \
            (oc) == OP_JUMP_IF  ? "JUMP_IF" :      \
            (oc) == OP_JUMP_IFNOT ? "JUMP_IFNOT" : \
            "UNKNOWN_OPCODE"                       \
    )

#define print_token(t) do{ printf("\033[34m%s\033[0m(\033[1m%s\033[0m)\n", token_to_str(t), t.literal ? t.literal : "(null)"); } while(0)
#define print_ast(n, i) print_node(n, i)
#define print_scope(scope, indent) print_scope(scope, indent)
#define print_symbol_table(st) print_symbol_table(st)
#define print_opcode(op) OPCODE_TO_STR(oc)

#endif
