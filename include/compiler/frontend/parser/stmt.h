#pragma once

#include "compiler/frontend/ast.h"      // node_t
#include "compiler/frontend/parser.h"   // parser_t

node_t* parse_stmt(parser_t* parser);
node_t* parse_stmt_keyword(parser_t* parser);
node_t* parse_stmt_block(parser_t* parser);
node_t* parse_stmt_jump(parser_t* parser);

bool add_stmt_block(parser_t* parser, node_t* block, node_t* stmt);

node_t* parse_stmt_if(parser_t* parser);
node_t* parse_stmt_while(parser_t* parser);
node_t* parse_stmt_for(parser_t* parser);
node_t* parse_stmt_case(parser_t* parser);
node_t* parse_stmt_match(parser_t* parser);
node_t* parse_stmt_trycatch(parser_t* parser);
