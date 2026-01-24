#pragma once

#include "compiler/frontend/ast.h"
#include "compiler/frontend/parser.h"

node_t* parse_decl_var(parser_t* parser);
node_t* parse_decl_array(parser_t* parser);
node_t* parse_decl_param(parser_t* parser);
node_t* parse_decl_member(parser_t* parser);
node_t* parse_decl_func(parser_t* parser);
node_t* parse_decl_struct(parser_t* parser);
node_t* parse_decl_enum(parser_t* parser);
node_t* parse_decl_trait(parser_t* parser);
node_t* parse_decl_type(parser_t* parser);
node_t* parse_decl_module(parser_t* parser);
node_t* parse_decl_import(parser_t* parser);
node_t* parse_decl_impl(parser_t* parser);
