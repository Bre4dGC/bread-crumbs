#include <stdio.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "core/common/filesystem.h"
#include "core/common/source.h"
#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser.h"
#include "compiler/frontend/semantic.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);
    arena_t* ast_arena = new_arena(ARENA_BIG_SIZE);
    string_pool_t lexer_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    string_pool_t parser_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    string_pool_t symbol_pool = new_string_pool(ARENA_DEFAULT_SIZE);
    report_table_t* reports = new_report_table(arena);

    if(!arena || !ast_arena || !reports){
        fprintf(stderr, "Failed to initialize core components\n");
        goto cleanup;
    }

    string_t input = fs_read_file(&lexer_pool, "test/cases/analisis.brc");
    if(!input.data){
        fprintf(stderr, "Failed to read input file\n");
        goto cleanup;
    }

    init_tokens();

    lexer_t* lexer = new_lexer(arena, &lexer_pool, reports, &input);
    if(!lexer){
        fprintf(stderr, "Failed to create lexer\n");
        goto cleanup;
    }

    semantic_t* sem = NULL;
    ast_t* ast = NULL;
    parser_t* parser = new_parser(arena, ast_arena, reports, &parser_pool, lexer);
    if(!parser){
        fprintf(stderr, "Failed to create parser\n");
        goto cleanup;
    }

    ast = parse_program(parser);
    if(!ast || !ast->nodes){
        fprintf(stderr, "Failed to parse program\n");
        print_report_table(reports);
        goto cleanup;
    }

    sem = new_semantic(arena, &symbol_pool, reports);
    if(!sem){
        fprintf(stderr, "Failed to create semantic analyzer\n");
        goto cleanup;
    }

    analyze_ast(sem, ast->nodes);
    print_report_table(reports);

cleanup:
    if(sem) free_semantic(sem);
    if(ast) free_ast(ast_arena);
    if(reports) free_report_table(reports);
    free_tokens();
    free_string_pool(&lexer_pool);
    free_string_pool(&parser_pool);
    free_string_pool(&symbol_pool);
    if(arena) free_arena(arena);

    bm_stop();
    bm_print("Test semantic analyzer");
    return 0;
}
