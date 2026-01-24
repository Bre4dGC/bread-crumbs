#pragma once

#include "compiler/frontend/ast.h"
#include "compiler/frontend/parser.h"

node_t* parse_expr(parser_t* parser);
node_t* parse_expr_operator(parser_t* parser);
node_t* parse_expr_keyword(parser_t* parser);
node_t* parse_expr_paren(parser_t* parser);
node_t* parse_expr_literal(parser_t* parser);

node_t* parse_expr_postfix(parser_t* parser);
node_t* parse_expr_unaryop(parser_t* parser);
node_t* parse_expr_binop(parser_t* parser, int min_precedence);
node_t* parse_expr_primary(parser_t* parser);
node_t* parse_expr_func_call(parser_t* parser);
node_t* parse_expr_var_ref(parser_t* parser);
