#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "core/arena.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser/expr.h"
#include "compiler/frontend/parser/decl.h"
#include "compiler/frontend/parser/stmt.h"

typedef node_t* (*parse_func_t)(parser_t*);

typedef struct {
    lexer_t* lexer;

    struct {
        token_t current;
        token_t next;
    } token;

    arena_t* ast;
    report_table_t* reports;
    string_pool_t* string_pool;
} parser_t;

parse_func_t parse_table[] = {
    [KW_IF]       = parse_stmt_if,
    [KW_WHILE]    = parse_stmt_while,
    [KW_FOR]      = parse_stmt_for,
    [KW_CASE]     = parse_stmt_case,
    [KW_MATCH]    = parse_stmt_match,
    [KW_TRY]      = parse_stmt_trycatch,
    [KW_BREAK]    = parse_stmt_jump,
    [KW_CONTINUE] = parse_stmt_jump,
    [KW_RETURN]   = parse_stmt_jump,
    [KW_TYPEOF]   = parse_stmt_special,
    [KW_NAMEOF]   = parse_stmt_special,

    [KW_FUNC]     = parse_decl_func,
    [KW_TRAIT]    = parse_decl_trait,
    [KW_IMPL]     = parse_decl_impl,
    [KW_TYPE]     = parse_decl_type,
    [KW_MODULE]   = parse_decl_module,
    [KW_IMPORT]   = parse_decl_import,
};

#define PARSE_TABLE_LENGTH sizeof(parse_table)/sizeof(parse_table[0])

parser_t* new_parser(arena_t* arena, arena_t* ast, report_table_t* reports, string_pool_t* string_pool, lexer_t* lexer);
ast_t* parse_program(parser_t* parser);

void advance_token(parser_t* parser);
bool consume_token(parser_t* parser, const enum category_tag expec_category, const int expec_type, const enum report_code err);

inline bool check_token(parser_t* parser, enum category_tag category, int type);
inline bool is_eof(const token_t token);
inline void set_node_location(node_t* node, parser_t* parser);
inline void set_node_length(node_t* node, parser_t* parser, size_t start_pos);
inline size_t get_lexer_position(parser_t* parser);