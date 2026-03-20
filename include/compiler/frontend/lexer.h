#pragma once

#include <stddef.h>     // size_t

#include "compiler/context.h"   // compiler_context_t
#include "core/lang/source.h"   // location_t
#include "compiler/frontend/lexer/tokens.h" // token_t

typedef struct {
    char ch;
    location_t loc;
    size_t balance;

    compiler_context_t* ctx;
} lexer_t;

lexer_t* new_lexer(compiler_context_t* ctx);
token_t next_token(lexer_t* lexer);
