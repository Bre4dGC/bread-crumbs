#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/frontend/parser.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();
    
    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);
    string_pool_t string_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    arena_t* ast_arena = new_arena(ARENA_BIG_SIZE);
    report_table_t* reports = new_report_table(arena);
    
    const char* code = "var x : int = 42";
    string_t input = new_string(&string_pool, code);
    

    init_tokens();
    
    lexer_t* lexer = new_lexer(arena, &string_pool, reports, &input);
    if(!lexer){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    parser_t* parser = new_parser(arena, ast_arena, reports, lexer);
    if(!parser){
        fprintf(stderr, "Failed to create parser\n");
        return 1;
    }

    ast_t* ast = parse_program(parser);

    free_tokens();
    free_ast(ast_arena);
    free_report_table(reports);
    free_string_pool(&string_pool);
    
    bench_stop();
    bench_print("Test parser");
    return 0;
}
