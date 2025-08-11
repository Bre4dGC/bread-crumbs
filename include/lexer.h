#pragma once

#include <stddef.h>
#include <wchar.h>
#include "tokenizer.h"

typedef struct {
    wchar_t *input;
    wchar_t ch;
    size_t pos;
    size_t nextpos;
    size_t line;
    size_t column;
    int paren_balance;
} Lexer;

Lexer* lex_new(const wchar_t *input);
void lex_free(Lexer *lexer);
Token tok_next(Lexer *lexer);