#include "ast.h"

struct ast_node* new_ast(enum node_type type)
{
    struct ast_node* node = (struct ast_node*)malloc(sizeof(struct ast_node));
    if (!node) return NULL;
    node->type = type;
    return node;
}

void free_ast(struct ast_node* node)
{
    if (node) free(node);
}