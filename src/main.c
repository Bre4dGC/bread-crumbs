#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "../include/lexer.h"
#include "../include/parser.h"

int repl_mode();
int run_mode(const char *src);
int compile_mode(const char *src);

int main(int argc, char *argv[])
{
    // interpreter
    if(strcmp(argv[1], "run")) {
        run_mode(argv[2]);
    }
    // JIT-compiler
    else if(strcmp(argv[1], "compile")) {
        compile_mode(argv[2]);
    }
    // single line
    else {
        repl_mode();
    }
    return 0;
}

int repl_mode()
{
    Lexer *lexer;
    Token *token = (Token *)malloc(sizeof(Token));
    while(1){
        wchar_t input[128];

        printf(">> ");
        fgetws(input, sizeof(input), stdin);

        if(input[0] == '\n') continue;
        else if(wcscmp(input, L":quit")) break;
        else {
            // TODO: implement executing
        }
    }
    lex_free(lexer);
    tok_free(token);
    return 0;
}

int run_mode(const char *src)
{
    // TODO: implement interpreter mode
    return 0;
}

int compile_mode(const char *src)
{
    printf("JIT-compiler not able yet :(");
    return 0;
}

int package_install(const char *pk_name)
{
    printf("Package Manager not able yet :(");
    return 0;
}
