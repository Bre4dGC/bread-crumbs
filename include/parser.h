#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ast.h"

struct parser {
    struct lexer *lexer;
    struct token tcurrent;
    struct token tnext;
};

struct parser* new_parser(struct lexer* lexer);
struct ast_node* parse_expr(struct parser* parser);
void free_parser(struct parser* parser);