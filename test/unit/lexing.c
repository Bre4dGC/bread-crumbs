#include "core/arena.h"
#include "core/diagnostic.h"
#include "core/strings.h"
#include "compiler/frontend/lexer.h"
#include "core/common/source.h"
#include "core/common/benchmark.h"

int main(void)
{
    bench_start();

    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);
    string_pool_t string_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    report_table_t* reports = new_report_table(arena);

    string_t input = read_file(&string_pool, "test/cases/tokens.brc");
    if(!input.data){
        fprintf(stderr, "Failed to read input file\n");
        return 1;
    }

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
