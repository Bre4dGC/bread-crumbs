#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "vm.h"
#include "errors.h"
#include "debug.h"

struct ast_node* new_ast(enum node_type type)
{
    struct ast_node* node = (struct ast_node*)calloc(1, sizeof(struct ast_node));
    if(!node) return NULL;
    node->type = type;
    node->line = 0;
    
    return node;
}

void compile_ast(struct ast_node* node, struct vm* vm)
{
    // TODO: implement
    #ifdef DEBUG
        print_node(node, 0);
    #endif
}

void free_ast(struct ast_node* node)
{
    if(!node) return;

    switch(node->type){
        case NODE_BIN_OP:
            free_ast(node->bin_op.left);
            free_ast(node->bin_op.right);
            break;

        case NODE_UNARY_OP:
            free_ast(node->unary_op.right);
            break;

        case NODE_VAR:
            if(node->var_decl){
                free(node->var_decl->name);
                free_ast(node->var_decl->value);
                free(node->var_decl);
            }
            break;

        case NODE_VAR_REF:
            if(node->var_ref.name) free(node->var_ref.name);
            break;

        case NODE_LITERAL:
            if(node->literal.value) free(node->literal.value);
            break;

        case NODE_BLOCK:
            for (size_t i = 0; i < node->block.count; i++){
                free_ast(node->block.statements[i]);
            }
            if(node->block.statements) free(node->block.statements);
            break;

        case NODE_FUNC_CALL:
            if(node->func_call.name) free(node->func_call.name);
            for(size_t i = 0; i < node->func_call.arg_count; i++){
                free_ast(node->func_call.args[i]);
            }
            if(node->func_call.args) free(node->func_call.args);
            break;

        case NODE_RETURN:
            if(node->return_stmt.body){
                free_ast(node->return_stmt.body);
            }
            break;

        case NODE_BREAK:
            break;

        case NODE_ARRAY:
            if(node->array_decl){
                for (size_t i = 0; i < node->array_decl->count; i++){
                    free_ast(node->array_decl->elements[i]);
                }
                if(node->array_decl->elements) free(node->array_decl->elements);
                free(node->array_decl);
            }
            break;

            case NODE_IF:
            if(node->if_stmt){
                free_ast(node->if_stmt->condition);
                free_ast(node->if_stmt->then_block);
                if(node->if_stmt->else_block){
                    free_ast(node->if_stmt->else_block);
                }
                if(node->if_stmt->elif_blocks){
                    free_ast(node->if_stmt->elif_blocks);
                }
                free(node->if_stmt);
            }
            break;

        case NODE_WHILE:
            if(node->while_loop){
                free_ast(node->while_loop->condition);
                free_ast(node->while_loop->body);
                free(node->while_loop);
            }
            break;

        case NODE_FOR:
            if(node->for_loop){
                free_ast(node->for_loop->init);
                free_ast(node->for_loop->condition);
                free_ast(node->for_loop->update);
                free_ast(node->for_loop->body);
                free(node->for_loop);
            }
            break;

        case NODE_FUNC:
            if(node->func_decl){
                if(node->func_decl->name) free(node->func_decl->name);
                for(size_t i = 0; i < node->func_decl->param_count; i++){
                    free_ast(node->func_decl->params[i]);
                }
                if(node->func_decl->params) free(node->func_decl->params);
                free_ast(node->func_decl->body);
                free(node->func_decl);
            }
            break;

        case NODE_STRUCT:
            if(node->struct_decl){
                if(node->struct_decl->name) free(node->struct_decl->name);
                for (size_t i = 0; i < node->struct_decl->member_count; i++){
                    free_ast(node->struct_decl->members[i]);
                }
                if(node->struct_decl->members) free(node->struct_decl->members);
                free(node->struct_decl);
            }
            break;

        case NODE_UNION:
            if(node->union_decl){
                if(node->union_decl->name) free(node->union_decl->name);
                for (size_t i = 0; i < node->union_decl->member_count; i++){
                    free_ast(node->union_decl->members[i]);
                }
                if(node->union_decl->members) free(node->union_decl->members);
                free(node->union_decl);
            }
            break;

        case NODE_ENUM:
            if(node->enum_decl){
                if(node->enum_decl->name) free(node->enum_decl->name);
                for (size_t i = 0; i < node->enum_decl->member_count; i++){
                    if(node->enum_decl->members[i]) free(node->enum_decl->members[i]);
                }
                if(node->enum_decl->members) free(node->enum_decl->members);
                free(node->enum_decl);
            }
            break;

        case NODE_MATCH:
            if(node->match_stmt){
                free_ast(node->match_stmt->target);
                for (size_t i = 0; i < node->match_stmt->case_count; i++){
                    free_ast(node->match_stmt->cases[i]);
                }
                if(node->match_stmt->cases) free(node->match_stmt->cases);
                free(node->match_stmt);
            }
            break;

        case NODE_CASE:
            if(node->match_case){
                free_ast(node->match_case->condition);
                free_ast(node->match_case->body);
                free(node->match_case);
            }
            break;

        case NODE_TRAIT:
            if(node->trait_decl){
                if(node->trait_decl->name) free(node->trait_decl->name);
                free_ast(node->trait_decl->body);
                free(node->trait_decl);
            }
            break;

        case NODE_TRYCATCH:
            if(node->trycatch_stmt){
                free_ast(node->trycatch_stmt->try_block);
                if(node->trycatch_stmt->catch_block) free_ast(node->trycatch_stmt->catch_block);
                if(node->trycatch_stmt->finally_block) free_ast(node->trycatch_stmt->finally_block);
                free(node->trycatch_stmt);
            }
            break;

        case NODE_IMPORT:
            if(node->import_stmt){
                if(node->import_stmt->module_name) free(node->import_stmt->module_name);
                free(node->import_stmt);
            }
            break;

        case NODE_TEST:
            if(node->test_stmt){
                if(node->test_stmt->name) free(node->test_stmt->name);
                free_ast(node->test_stmt->body);
                free(node->test_stmt);
            }
            break;

        case NODE_FORK:
            if(node->fork_stmt){
                if(node->fork_stmt->name) free(node->fork_stmt->name);
                free_ast(node->fork_stmt->body);
                free(node->fork_stmt);
            }
            break;

        case NODE_SOLVE:
            if(node->solve_stmt){
                for (size_t i = 0; i < node->solve_stmt->param_count; i++){
                    free_ast(node->solve_stmt->params[i]);
                }
                if(node->solve_stmt->params) free(node->solve_stmt->params);
                free_ast(node->solve_stmt->body);
                free(node->solve_stmt);
            }
            break;

        case NODE_SIMULATE:
            if(node->simulate_stmt){
                free_ast(node->simulate_stmt->body);
                free(node->simulate_stmt);
            }
            break;

        default:
            break;
    }
    free(node);
}
