#include <stdio.h>
#include <sys/time.h>

#include "compiler/lexer.h"

int main(void)
{
    const char *inputs[] = {
        "func main() => int { return 0; }",
        "if (x < 10) { x += 1; } else { x = 0; }",
        "var x = 42; # This is a comment",
        "var name: str = \"bread",
    };

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        printf("%zu: %s\n\n", i+1, inputs[i]);

        struct lexer *lex = new_lexer(inputs[i]);
        if(!lex) {
            fprintf(stderr, "Failed to create lexer\n");
            return 1;
        }

        struct timeval start, end;
        gettimeofday(&start, NULL);

        struct token token;
        int total_tokens = 0;
        while((token = next_token(lex)).category != CATEGORY_SERVICE || token.type_service != SERV_EOF) {
            free_token(&token);
            total_tokens++;
        }
    
        gettimeofday(&end, NULL);
        double microseconds = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
        
        printf("Processed in %.2f microseconds\n", microseconds);
        printf("Total tokens: %d\n", total_tokens);
        printf("Average time per token: %f microseconds\n", microseconds / total_tokens);

        for(size_t i = 0; i < lex->errors_count; ++i){
            print_error(lex->errors[i]);
        }

        free_token(&token);
        free_lexer(lex);

        printf("_________________________________\n");
    }

    return 0;
}
