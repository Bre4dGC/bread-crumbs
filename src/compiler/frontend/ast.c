#include "compiler/frontend/ast.h"

node_t* new_node(arena_t* arena, enum node_kind kind)
{
    node_t* node = (node_t*)arena_alloc(arena, sizeof(node_t), alignof(node_t));
    if(!node) return NULL;
    node->kind = kind;
    return node;
}

void free_ast(arena_t* root)
{
    if(!root) return;
    free_arena(root);
}
