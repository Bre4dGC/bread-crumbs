#pragma once
#include <stddef.h>

#include "core/diagnostic.h"
#include "core/strings.h"
#include "compiler/frontend/lexer/tokens.h"

typedef struct {
    string_t* input;

    char ch;
    size_t pos;
    location_t loc;
    size_t balance;

    string_pool_t* string_pool;
    report_table_t* reports;
} lexer_t;

lexer_t* new_lexer(arena_t* arena, string_pool_t* string_pool, report_table_t* reports, string_t* input);
token_t next_token(lexer_t* lexer);
