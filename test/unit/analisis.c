#include <stdio.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser.h"
#include "compiler/frontend/semantic.h"
#include "core/common/filesystem.h"
#include "core/common/source.h"
#include "core/common/benchmark.h"

int main(void)
{
    bench_start();

    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);
    arena_t* ast_arena = new_arena(ARENA_BIG_SIZE);
    string_pool_t lexer_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    string_pool_t parser_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    string_pool_t symbol_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    report_table_t* reports = new_report_table(arena);

    string_t input = fs_read_file(&lexer_pool, "test/cases/analisis.brc");
    if(!input.data){
        fprintf(stderr, "Failed to read input file\n");
        return 1;
    }

    init_tokens();

    lexer_t* lexer = new_lexer(arena, &lexer_pool, reports, &input);
    if(!lexer){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    parser_t* parser = new_parser(arena, ast_arena, reports, &parser_pool, lexer);
    if(!parser){
        fprintf(stderr, "Failed to create parser\n");
        return 1;
    }

    ast_t* ast = parse_program(parser);
    if(!ast || !ast->nodes){
        fprintf(stderr, "Failed to parse program\n");
        print_report_table(reports);
        return 1;
    }

    semantic_t* sem = new_semantic(arena, &symbol_pool, reports);
    if(!sem){
        fprintf(stderr, "Failed to create semantic analyzer\n");
        return 1;
    }

    analyze_ast(sem, ast->nodes);

    print_report_table(reports);

    free_semantic(sem);
    if(ast) free_ast(ast_arena);
    free_report_table(reports);
    free_string_pool(&lexer_pool);
    free_string_pool(&parser_pool);
    free_string_pool(&symbol_pool);
    free_arena(arena);
    free_tokens();

    bench_stop();
    bench_print("Test semantic analyzer");
    return 0;
}
