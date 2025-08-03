#include <wchar.h>
#include <stdio.h>

#include "lexer.h"

int main(void)
{
    const wchar_t *inputs[] = {
        L"var 123abc = 42\n",  // TODO: fix identifiers starting with digits
        L"var x = 0xGHI\n",  // TODO: fix hexadecimal and binary parsing
        L"list<int32> numbers = [1, 2, , 3]\n",  // TODO: fix list parsing
        // TODO: add more test cases
    };

    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        wprintf(L"%zu: %ls\n", i + 1, inputs[i]);

        Lexer *lexer = lex_new(inputs[i]);
        if (!lexer) {
            wprintf(L"Failed to create lexer\n");
            continue;
        }

        Token token;
        while ((token = tok_next(lexer)).tag != TYPE_SERVICE || token.service != T_EOF) {
            tok_free(&token);
        }
        lex_free(lexer);
        
        wprintf(L"_________________________________\n");
    }

    return 0;
}