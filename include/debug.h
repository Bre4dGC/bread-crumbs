#pragma once

#ifdef DEBUG

#include <stdio.h>
#include <stdbool.h>

#include "lexer.h"
#include "vm.h"
#include "ast.h"

static inline const char* type_to_str(const struct token* tok)
{
    if (!tok) return "NULL";
    switch(tok->category){
        case CATEGORY_SERVICE:
            switch(tok->type_service){
                case SERV_ILLEGAL: return "ILLEGAL";
                default: return "SERVICE";
            }
        case CATEGORY_LITERAL:
            switch(tok->type_literal){
                case LIT_IDENT: return "IDENT";
                case LIT_STRING: return "STRING";
                case LIT_TRUE: case LIT_FALSE: return "BOOLEAN";
                default: return "NUMBER";
            }
        case CATEGORY_DATATYPE: return "DATATYPE";
        case CATEGORY_KEYWORD: return "KEYWORD";
        case CATEGORY_DELIMITER: return "DELIMITER";
        case CATEGORY_OPERATOR: return "OPERATOR";
        case CATEGORY_MODIFIER: return "MODIFIER";
        case CATEGORY_PAREN: return "PAREN";
        default: return "UNKNOWN";
    }
}

static inline const char* opcode_to_str(enum op_code oc);

static inline void ast_print(struct ast_node* node, int indent)
{
    if (!node) return;

    for (int i = 0; i < indent; i++){
        printf("  ");
    }
    
    switch (node->type){
        case NODE_LITERAL: printf("Literal: %s\n", node->literal.value);
            break;            
        case NODE_VAR_REF: printf("VarRef: %s\n", node->var_ref.name);
            break;            
        case NODE_BIN_OP: 
            printf("BinOp: %s\n", opcode_to_str(node->bin_op.code));
            ast_print(node->bin_op.left, indent + 1);
            ast_print(node->bin_op.right, indent + 1);
            break;            
        case NODE_VAR:
            printf("VarDecl: %s\n", node->var_decl->name);
            ast_print(node->var_decl->value, indent + 1);
            break;            
        case NODE_BLOCK:
            printf("Block:\n");
            for (size_t i = 0; i < node->block.count; i++){
                ast_print(node->block.statements[i], indent + 1);
            }
            break;
        case NODE_UNARY_OP:
            printf("UnaryOp: %s\n", opcode_to_str(node->unary_op.code));
            ast_print(node->unary_op.right, indent + 1);
            break;
        case NODE_FUNC_CALL:
            printf("FuncCall: %s\n", node->func_call.name);
            for (size_t i = 0; i < node->func_call.arg_count; i++){
                ast_print(node->func_call.args[i], indent + 1);
            }
            break;
        case NODE_RETURN:
            printf("Return:\n");
            ast_print(node->return_stmt.body, indent + 1);
            break;
        case NODE_ARRAY:
            printf("Array:\n");
            for (size_t i = 0; i < node->array_decl->count; i++){
                ast_print(node->array_decl->elements[i], indent + 1);
            }
            break;
        case NODE_IF:
            printf("If Statement:\n");
            ast_print(node->if_stmt->condition, indent + 1);
            ast_print(node->if_stmt->then_block, indent + 1);
            if (node->if_stmt->elif_blocks) {
                ast_print(node->if_stmt->elif_blocks, indent + 1);
            }
            if (node->if_stmt->else_block) {
                ast_print(node->if_stmt->else_block, indent + 1);
            }
            break;
        case NODE_WHILE:
            printf("While Loop:\n");
            ast_print(node->while_loop->condition, indent + 1);
            ast_print(node->while_loop->body, indent + 1);
            break;
        case NODE_FOR:
            printf("For Loop:\n");
            ast_print(node->for_loop->init, indent + 1);
            ast_print(node->for_loop->condition, indent + 1);
            ast_print(node->for_loop->update, indent + 1);
            ast_print(node->for_loop->body, indent + 1);
            break;
        case NODE_FUNC:
            printf("Function: %s\n", node->func_decl->name);
            for (size_t i = 0; i < node->func_decl->param_count; i++){
                ast_print(node->func_decl->params[i], indent + 1);
            }
            ast_print(node->func_decl->body, indent + 1);
            break;
        case NODE_STRUCT:
            printf("Struct: %s\n", node->struct_decl->name);
            for (size_t i = 0; i < node->struct_decl->member_count; i++){
                ast_print(node->struct_decl->members[i], indent + 1);
            }
            break;
        case NODE_UNION:
            printf("Union: %s\n", node->union_decl->name);
            for (size_t i = 0; i < node->union_decl->member_count; i++){
                ast_print(node->union_decl->members[i], indent + 1);
            }
            break;
        case NODE_ENUM:
            printf("Enum: %s\n", node->enum_decl->name);
            for (size_t i = 0; i < node->enum_decl->member_count; i++){
                printf("  %s\n", node->enum_decl->members[i]);
            }
            break;
        case NODE_MATCH:
            printf("Match Statement:\n");
            ast_print(node->match_stmt->target, indent + 1);
            for (size_t i = 0; i < node->match_stmt->case_count; i++){
                ast_print(node->match_stmt->cases[i], indent + 1);
            }
            break;
        case NODE_CASE:
            printf("Case:\n");
            ast_print(node->match_case->condition, indent + 1);
            ast_print(node->match_case->body, indent + 1);
            break;
        case NODE_TRAIT:
            printf("Trait: %s\n", node->trait_decl->name);
            ast_print(node->trait_decl->body, indent + 1);
            break;
        case NODE_TRYCATCH:
            printf("Try-Catch:\n");
            ast_print(node->trycatch_stmt->try_block, indent + 1);
            if (node->trycatch_stmt->catch_block) {
                ast_print(node->trycatch_stmt->catch_block, indent + 1);
            }
            if (node->trycatch_stmt->finally_block) {
                ast_print(node->trycatch_stmt->finally_block, indent + 1);
            }
            break;
        case NODE_IMPORT:
            printf("Import: %s\n", node->import_stmt->module_name);
            break;
        case NODE_TEST:
            printf("Test: %s\n", node->test_stmt->name);
            ast_print(node->test_stmt->body, indent + 1);
            break;
        case NODE_FORK:
            printf("Fork: %s\n", node->fork_stmt->name ? node->fork_stmt->name : "(anonymous)");
            ast_print(node->fork_stmt->body, indent + 1);
            break;
        case NODE_SOLVE:
            printf("Solve:\n");
            for (size_t i = 0; i < node->solve_stmt->param_count; i++){
                ast_print(node->solve_stmt->params[i], indent + 1);
            }
            ast_print(node->solve_stmt->body, indent + 1);
            break;
        case NODE_SIMULATE:
            printf("Simulate:\n");
            ast_print(node->simulate_stmt->body, indent + 1);
            break;
        case NODE_EXPR: 
        case NODE_ASSIGN:
            printf("Expression / Assignment (Unhandled display for now)");
            break;
    }
}

#define OPCODE_TO_STR(oc) \
    _Generic((oc), \
        enum op_code: \
            (oc) == OP_PUSH ? "OP_PUSH" : \
            (oc) == OP_POP ? "OP_POP" : \
            (oc) == OP_DUP ? "OP_DUP" : \
            (oc) == OP_ADD ? "OP_ADD" : \
            (oc) == OP_SUB ? "OP_SUB" : \
            (oc) == OP_MUL ? "OP_MUL" : \
            (oc) == OP_DIV ? "OP_DIV" : \
            (oc) == OP_AND ? "OP_AND" : \
            (oc) == OP_OR ? "OP_OR" : \
            (oc) == OP_NOT ? "OP_NOT" : \
            (oc) == OP_EQ ? "OP_EQ" : \
            (oc) == OP_NEQ ? "OP_NEQ" : \
            (oc) == OP_LT ? "OP_LT" : \
            (oc) == OP_GT ? "OP_GT" : \
            (oc) == OP_STORE ? "OP_STORE" : \
            (oc) == OP_LOAD ? "OP_LOAD" : \
            (oc) == OP_STORE_GLOB ? "OP_STORE_GLOB" : \
            (oc) == OP_LOAD_GLOB ? "OP_LOAD_GLOB" : \
            (oc) == OP_JUMP ? "OP_JUMP" : \
            (oc) == OP_CALL ? "OP_CALL" : \
            (oc) == OP_RETURN ? "OP_RETURN" : \
            (oc) == OP_JUMP_IF ? "OP_JUMP_IF" : \
            (oc) == OP_JUMP_IFNOT ? "OP_JUMP_IFNOT" : \
            "UNKNOWN_OPCODE" \
    )

static inline const char* opcode_to_str(enum op_code oc)
{
    return OPCODE_TO_STR(oc);
}

#define print_token(t) do{ if (t) printf("\033[34m%s\033[0m(\033[1m%s\033[0m)\n", type_to_str(t), (t)->literal ? (t)->literal : ""); } while(0)
#define print_node(n, i) ast_print(n, i)
#define print_opcode(op) opcode_to_str(op)

#endif
