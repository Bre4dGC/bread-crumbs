#pragma once
#include <stddef.h>

#include "compiler/frontend/tokenizer.h"
#include "compiler/core/diagnostic.h"

typedef struct {
    char* input;
    size_t input_len;
    
    char ch;

    size_t pos;
    size_t nextpos;
    size_t line;
    size_t column;

    int paren_balance;

    report_t** errors;
    size_t errors_count;
} lexer_t;

lexer_t* new_lexer(const char* input);
void free_lexer(lexer_t* lex);
token_t next_token(lexer_t* lex);
