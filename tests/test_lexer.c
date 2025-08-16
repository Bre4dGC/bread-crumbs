#include <wchar.h>
#include <stdio.h>

#include "lexer.h"

int main(void)
{
    const wchar_t *inputs[] = {
        L"var 123abc = 42",
        L"var line: str = \"string",
    };

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        // wprintf(L"1: %ls\n", inputs[i]);

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

        wprintf(L"_________________________________\n");
    }

    return 0;
}