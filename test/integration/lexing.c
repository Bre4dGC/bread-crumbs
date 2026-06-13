#include <assert.h>

#include "compiler/context.h"
#include "compiler/frontend/lexer.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    compiler_context_t* ctx = new_compiler_context();
    assert(ctx);

    lexer_t* lexer = new_lexer(ctx);
    assert(lexer);

    token_t token;
    do token = next_token(lexer); while(token.category != CAT_SERVICE || token.type != SERV_EOF);

    free_compiler_context(ctx);

    bm_stop();
    bm_print("Test lexer");

    return 0;
}
