#include <stdio.h>

#include "lexer.h"

int main(void)
{
    const char *inputs[] = {
        "var abd3c: bool = true;;;",
    };

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        printf("%zu: %s\n\n", i+1, inputs[i]);

        struct lexer *lex = new_lexer(inputs[i]);
        if(!lex) {
            fprintf(stderr, "Failed to create lexer\n");
            return 1;
        }

        struct token token;
        while((token = next_token(lex)).category != CATEGORY_SERVICE || token.type_service != SERV_EOF) {
            free_token(&token);
        }

        for(size_t i = 0; i < lex->errors_count; ++i){
            print_error(lex->errors[i]);
        }

        free_token(&token);
        free_lexer(lex);

        printf("_________________________________\n");
    }

    return 0;
}
