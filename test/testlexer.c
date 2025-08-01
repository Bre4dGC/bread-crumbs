#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

int main(void)
{
    // open the main.bc file
    // read the contents into a wide character string
    // FILE *file = fopen("main.txt", "r");
    // if (!file) {
    //     wprintf(L"Failed to open main.bc\n");
    //     return 1;
    // }
    // fseek(file, 0, SEEK_END);
    // long file_size = ftell(file);
    // fseek(file, 0, SEEK_SET);
    // wchar_t *input = malloc((file_size / sizeof(wchar_t) + 1) * sizeof(wchar_t));
    // if (!input) {
    //     fclose(file);
    //     wprintf(L"Failed to allocate memory for input\n");
    //     return 1;
    // }
    // fread(input, sizeof(wchar_t), file_size / sizeof(wchar_t), file);
    // fclose(file);

    const wchar_t input[] = L"var name: str = \"Hello, World!\"\
                              if(2 == 2) {\
                                  dosmth()\
                              }";

    Lexer *lexer = lex_new(input);
    if (!lexer) {
        wprintf(L"Failed to create lexer\n");
        return 1;
    }
    Token token;
    while ((token = tok_next(lexer)).tag != TYPE_SERVICE || token.service != T_EOF) {
        tok_free(&token);
    }
    wprintf(L"End of input reached\n");
    lex_free(lexer);
    return 0;
}
