#include "ast.h"

struct ast_node* new_ast(enum node_type type)
{
    struct ast_node* node = (struct ast_node*)calloc(1, sizeof(struct ast_node));
    if (!node) return NULL;
    node->type = type;
    return node;
}

void free_ast(struct ast_node* node)
{
    if (!node) return;    
    /* TODO: implement recursive deep-free based on node->type:
       - free strings (e.g. func_call.name, var_decl->name, literal.str_val)
       - free child ast nodes recursively
       - free node->var_decl / node->func_decl structures and their internals
    */

    free(node);
}