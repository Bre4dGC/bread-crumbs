#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "compiler/frontend/ast.h"
#include "compiler/core/diagnostic.h"

typedef struct {
    lexer_t* lexer;
    token_t current;
    token_t peek;
    report_t** errors;
    size_t errors_count;
} parser_t;

parser_t* new_parser(lexer_t* lexer);
astnode_t* parse_program(parser_t* pars);
void free_parser(parser_t* parser);
