#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "core/arena.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/lexer.h"

typedef struct parser parser_t;
typedef node_t* (*parse_func_t)(parser_t*);

struct parser {
    lexer_t* lexer;

    struct {
        token_t current;
        token_t next;
    } token;

    arena_t* ast;
    report_table_t* reports;
    string_pool_t* string_pool;
};

extern parse_func_t parse_table[];
extern const size_t PARSE_TABLE_LENGTH;

parser_t* new_parser(arena_t* arena, arena_t* ast, report_table_t* reports, string_pool_t* string_pool, lexer_t* lexer);
ast_t* parse_program(parser_t* parser);

void advance_token(parser_t* parser);
bool consume_token(parser_t* parser, const enum category_tag expec_category, const int expec_type, const enum report_code err);

bool check_token(parser_t* parser, enum category_tag category, int type);
bool is_eof(const token_t token);
void set_node_location(node_t* node, parser_t* parser);
void set_node_length(node_t* node, parser_t* parser, size_t start_pos);
size_t get_lexer_position(parser_t* parser);
