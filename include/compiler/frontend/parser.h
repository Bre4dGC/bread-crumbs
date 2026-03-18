#pragma once

#include <stdint.h>     // uint8_t, bool
#include <stdbool.h>    // bool
#include <stddef.h>     // size_t

#include "compiler/context.h"           // compiler_context_t
#include "compiler/frontend/lexer.h"    // lexer_t

typedef struct parser parser_t;
typedef node_t* (*parse_func_t)(parser_t*);

struct parser {
    lexer_t* lexer;

    struct {
        token_t current;
        token_t next;
    } token;

    compiler_context_t* ctx;
};

extern parse_func_t parse_table[];
extern const size_t PARSE_TABLE_LENGTH;

parser_t* new_parser(compiler_context_t* ctx, lexer_t* lexer);
ast_t* parse_program(parser_t* parser);

void advance_token(parser_t* parser);
bool consume_token(parser_t* parser, node_t* node, const enum category_tag expec_category, const int expec_type, const enum report_code err);
bool check_token(parser_t* parser, enum category_tag category, int type);
bool is_eof(const token_t token);

void set_node_loc(node_t* node, parser_t* parser);
void set_node_len(node_t* node, parser_t* parser, size_t start_pos);
size_t get_lexer_pos(parser_t* parser);
