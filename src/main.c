#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"
#include "../include/parser.h"

int repl_mode();
int run_mode(const char *src);

int main(int argc, char *argv[])
{
    if(strcmp(argv[1], "run")) {
        run_mode(argv[2]);
    }
    else if(strcmp(argv[1], "compile")) {
        run_mode(argv[2]);
    }
    else if(strcmp(argv[1], "install")) {
        run_mode(argv[2]);
    }
    else {
        repl_mode();
    }

    return 0;
}

int repl_mode()
{
    const char prompt[] = ">> ";
}

int run_mode(const char *src)
{

}
