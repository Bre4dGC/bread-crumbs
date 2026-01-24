#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/core/string_pool.h"
#include "compiler/frontend/lexer.h"
#include "common/benchmark.h"

int main(void)
{    
    bench_start();

    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);
    string_pool_t string_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    report_table_t* reports = new_report_table(arena);

    const char* code = "++ -- == != += -= *= /= %= && || <= >= .. -> if else elif for do while func return break continue default match case struct enum import module type trait impl try catch finally async await yield test assert verify solve where snapshot rollback commit fork merge revert push pull clone simulate choose scenarios branch int uint short ushort long ulong char byte float decimal str bool any var const final static true false null infinity nameof typeof";
    string_t input = new_string(&string_pool, code);
    
    lexer_t* lexer = new_lexer(arena, &string_pool, reports, &input);
    if(!lexer){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    init_tokens();

    token_t token;
    do token = next_token(lexer); while(!(token.category == CAT_SERVICE && token.type == SERV_EOF));

    print_report_table(reports);

    free_tokens();
    free_report_table(reports);
    free_string_pool(&string_pool);
    free_arena(arena);

    bench_stop();
    bench_print("Test new lexer");

    return 0;
}
