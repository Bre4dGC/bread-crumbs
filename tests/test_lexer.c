#include <stdio.h>
#include <sys/time.h>

#include "compiler/frontend/lexer.h"
#include "common/file_reader.h"
#include "common/utils.h"

char* filepath;

int main(void)
{
    filepath = util_strdup("tests/cases/shit.brc");
    
    size_t file_size = 0;
    char* source = read_file(filepath, &file_size);
    if(!source) return EXIT_FAILURE;

    struct lexer *lex = new_lexer(source);
    if(!lex){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    struct token token;
    int total_tokens = 0;
    while((token = next_token(lex)).category != CATEGORY_SERVICE || token.type_service != SERV_EOF){
        free_token(&token);
        total_tokens++;
    }

    gettimeofday(&end, NULL);
    double microseconds = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);

    printf("Processed in %.2f microseconds\n", microseconds);
    printf("Total tokens: %d\n", total_tokens);
    printf("Average time per token: %f microseconds\n", microseconds / total_tokens);

    for(size_t i = 0; i < lex->errors_count; ++i){
        print_report(lex->errors[i]);
    }

    free_token(&token);
    free_lexer(lex);
    free(source);
    free(filepath);

    return EXIT_SUCCESS;
}
