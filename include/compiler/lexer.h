#pragma once
#include <stddef.h>

#include "compiler/lexer/tokenizer.h"
#include "compiler/errors.h"

struct lexer {
    char* input;
    size_t input_len;
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
void free_lexer(struct lexer* lex);
struct token next_token(struct lexer* lex);
