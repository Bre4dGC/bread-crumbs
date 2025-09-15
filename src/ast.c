#include "ast.h"

struct ast_node* new_ast(enum node_type type)
{
    struct ast_node* node = (struct ast_node*)calloc(1, sizeof(struct ast_node));
    if (!node) return NULL;
    node->type = type;
    return node;
}

void compile_ast(struct ast_node* node, struct vm* vm)
{
    
}

void free_ast(struct ast_node* node)
{
    if (!node) return;

    switch (node->type) {
        case NODE_BIN_OP:
            free_ast(node->bin_op.left);
            free_ast(node->bin_op.right);
            break;
        case NODE_UNARY_OP:
            free_ast(node->unary_op.operand);
            break;
        case NODE_VAR:
            if (node->var_decl) {
                free(node->var_decl->name);
                free_ast(node->var_decl->value);
                free(node->var_decl);
            }
            break;
        case NODE_VAR_REF:
            free(node->var_assign.name);
            free_ast(node->var_assign.value);
            break;
        case NODE_FUNC_CALL:
            free(node->func_call.name);
            for (size_t i = 0; i < node->func_call.arg_count; i++) {
                free_ast(node->func_call.args[i]);
            }
            free(node->func_call.args);
            break;
        case NODE_FUNC:
            if (node->func_decl) {
                free(node->func_decl->name);
                for (size_t i = 0; i < node->func_decl->param_count; i++) {
                    free_ast(node->func_decl->params[i]);
                }
                free(node->func_decl->params);
                free_ast(node->func_decl->body);
                free(node->func_decl);
            }
            break;
        case NODE_IF:
            free_ast(node->if_stmt->condition);
            free_ast(node->if_stmt->then_block);
            if (node->if_stmt->else_block) {
                free_ast(node->if_stmt->else_block);
            }
            break;
        case NODE_WHILE:
            free_ast(node->while_loop->condition);
            free_ast(node->while_loop->body);
            break;
        case NODE_RETURN:
            if (node->return_stmt.body) {
                free_ast(node->return_stmt.body);
            }
            break;
        case NODE_BLOCK:
            
            break;
        default:
            break;
    }
    free(node);
}
