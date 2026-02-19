#include "compiler/frontend/ast/visitor.h"

ast_visitor_t* new_ast_visitor(void) {
    ast_visitor_t* visitor = malloc(sizeof(ast_visitor_t));
    visitor->arena = arena_create();
    visitor->string_table = hashmap_create();
    memset(visitor->visit_table, 0, sizeof(visitor->visit_table));
    return visitor;
}

void free_ast_visitor(ast_visitor_t* visitor)
{
    // TODO: implement
}

void ast_visit(ast_visitor_t* visitor, ast_t* ast)
{
    // TODO: implement
}

