#pragma once
#include <stddef.h>

#include "compiler/core/diagnostic.h"
#include "compiler/core/string_pool.h"
#include "compiler/frontend/tokenizer.h"

typedef struct {
    string_t* input;

    char ch;
    size_t pos;
    location_t loc;

    size_t balance;
    size_t boln;

    string_pool_t* string_pool;
    report_table_t* reports;
} lexer_t;

lexer_t* new_lexer(arena_t* arena, string_pool_t* string_pool, report_table_t* reports, string_t* input);
token_t next_token(lexer_t* lexer);
