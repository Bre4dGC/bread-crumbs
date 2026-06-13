# Parser - Documentation

This document provides an overview of the parser component of the compiler, which is responsible for analyzing the token stream produced by the lexer and constructing an abstract syntax tree (AST) that represents the structure of the source code.

## Grammar

## Syntax

## AST

## Parser

```c
struct parser_t {
    lexer_t* lexer;

    struct {
        token_t current;
        token_t next;
    } token;

    compiler_context_t* ctx;
};

typedef node_t* (*parse_func_t)(parser_t*);

extern parse_func_t parse_table[];
extern const size_t PARSE_TABLE_LENGTH;

parser_t* new_parser(compiler_context_t* ctx, lexer_t* lexer);
ast_t* parse_program(parser_t* parser);

void advance_token(parser_t* parser);
bool consume_token(parser_t* parser, node_t* node, const enum category_tag expect_category, const int expec_type, const enum report_code err);
bool check_token(parser_t* parser, enum category_tag category, int type);
bool is_eof(const token_t token);

void set_node_loc(node_t* node, parser_t* parser);
void set_node_len(node_t* node, parser_t* parser, size_t start_pos);
size_t get_lexer_pos(parser_t* parser);
```