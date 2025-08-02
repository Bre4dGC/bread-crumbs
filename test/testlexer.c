#include <wchar.h>
#include <stdio.h>

#include "lexer.h"

int main(void)
{
    const wchar_t *input = L"\nfunc calculate(x: int, y: int) -> float {\n"
                           L"\tvar result: float = x * y + 3.14\n"
                           L"\tif result > 100 {\n"
                           L"\t\treturn result / 2\n"
                           L"\t} else {\n"
                           L"\t\treturn result\n"
                           L"\t}\n"
                           L"} # End of function\n";
    wprintf(L"Input: %ls\n", input);

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