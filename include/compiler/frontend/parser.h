#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/lexer.h"

typedef struct {
    lexer_t* lexer;

    struct {
        token_t current;
        token_t next;
    } token;

    arena_t* ast;
    report_table_t* reports;
} parser_t;

parser_t* new_parser(arena_t* arena, arena_t* ast, report_table_t* reports, lexer_t* lexer);
ast_t* parse_program(parser_t* parser);
