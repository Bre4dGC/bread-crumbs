#include <stdio.h>

#include "compiler/frontend/lexer.h"
#include "common/utils.h"
#include "common/file_reader.h"
#include "common/benchmark.h"

char* filepath;

int main(void)
{    
    filepath = util_strdup("tests/cases/shit.brc");
    
    size_t file_size = 0;
    char* source = read_file(filepath, &file_size);
    if(!source) return EXIT_FAILURE;
    
    bench_start();

    lexer_t *lex = new_lexer(source);
    if(!lex){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    token_t token;
    int total_tokens = 0;
    while((token = next_token(lex)).category != CATEGORY_SERVICE || token.type_service != SERV_EOF){
        free_token(&token);
        total_tokens++;
    }

    for(size_t i = 0; i < lex->errors_count; ++i){
        print_report(lex->errors[i]);
    }
    
    bench_stop();
    bench_print();

    free_token(&token);
    free_lexer(lex);
    free(source);
    free(filepath);

    return EXIT_SUCCESS;
}
