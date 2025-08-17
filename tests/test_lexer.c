#include <stdio.h>

#include "lexer.h"

int main(void)
{
    const char *inputs[] = {
        "var abc = 0b101"
    };

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        printf("%d: %s\n\n", i+1, inputs[i]);

        struct lexer *lex = new_lexer(inputs[i]);
        if(!lex) {
            fprintf(stderr, "Failed to create lexer\n");
            return 1;
        }

        struct token tok;
        while((tok = next_token(lex)).category != CATEGORY_SERVICE || tok.service != SERV_EOF) {
            free_token(&tok);
        }

        for(int i = 0; i < lex->errors_count; ++i){
            print_error(lex->errors[i]);
        }

        free_token(&tok);
        free_lexer(lex);

        printf("_________________________________\n");
    }

    return 0;
}