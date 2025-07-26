#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/lexer.h"
#include "../include/parser.h"

int repl_mode();
int run_mode(const char *src);

int main(int argc, char *argv[])
{
    if(strcmp(argv[1], "run")) {
        run_mode(argv[2]);
    }

    // for JIT-compiler
    else if(strcmp(argv[1], "compile")) {
        printf("JIT-compiler not able yet :(");
    }

    // for package manager
    else if(strcmp(argv[1], "install")) {
        printf("Package Manager not able yet :(");
    }

    else {
        repl_mode();
    }

    return 0;
}

int repl_mode()
{
    while(1){
        char input[128];

        printf(">> ");
        fgets(input, sizeof(input), stdin);

        if(input[0] == '\n') continue;
        else if(strcmp(input, "#quit")) break;
        else if(strcmp(input, "#run")) {
            // TODO: implement run the code command
        }
        else if(strcmp(input, "#clear")) {
            // TODO: implement the clear repl command
        }
        else {
            Lexer *lexer = lex_new(input);
            Token *token = (Token *)malloc(sizeof(Token));
            for(int i = 0; token->service != T_EOF; ++i){
                token[i] = tok_next(lexer);
                token = (Token *)realloc(token, sizeof(Token *));
            }
            lex_free(lexer);
            tok_free(token);
        }
    }
}

int run_mode(const char *src)
{
    // TODO: implement interpreter mode
}
