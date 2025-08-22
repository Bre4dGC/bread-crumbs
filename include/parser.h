#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ast.h"
#include "errors.h"

struct parser {
    struct lexer *lexer;
    struct token tok_current;
    struct token tok_next;
    struct error **errors;
    size_t errors_count;
};

struct parser* new_parser(struct lexer* lexer);
struct ast_node* parse_expr(struct parser* parser);
void free_parser(struct parser* parser);