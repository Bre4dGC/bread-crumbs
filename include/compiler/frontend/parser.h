#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler/frontend/ast.h"
#include "compiler/diagnostic/errors.h"

struct parser {
    struct lexer* lexer;
    struct token current;
    struct token peek;
    struct error** errors;
    size_t errors_count;
};

struct parser* new_parser(struct lexer* lexer);
struct ast_node* parse_program(struct parser* pars);
void free_parser(struct parser* parser);
