#pragma once
#include <stddef.h>

#include "tokenizer.h"
#include "errors.h"

struct lexer {
    char *input;
    char ch;
    size_t pos;
    size_t nextpos;
    size_t line;
    size_t column;
    int paren_balance;
    struct error** errors;
    size_t errors_count;
};

struct lexer* new_lexer(const char* input);
void new_lexer_error(struct lexer* lex, struct error* err);
void free_lexer(struct lexer* lex);
struct token next_token(struct lexer* lex);
