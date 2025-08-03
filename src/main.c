#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>

#include "../include/lexer.h"
#include "../include/parser.h"

int repl_mode();
int run_mode(const char *src);
int compile_mode(const char *src);

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    
    // check for REPL mode
    if(argc < 2) {
        return repl_mode();
    }

    // check for help mode
    if(argc == 2 && strcmp(argv[1], "help") == 0) {
        fprintf(stderr, "Usage: %s <mode> [source]\n", argv[0]);
        fprintf(stderr, "Modes:\n");
        fprintf(stderr, "  run <source>    - Run the interpreter with the source file\n");
        fprintf(stderr, "  compile <source> - Compile the source file with JIT\n");
        fprintf(stderr, "  (no arguments)   - Start REPL mode\n");
        return EXIT_SUCCESS;
    }

    // check for valid mode
    if(strcmp(argv[1], "run") && strcmp(argv[1], "compile")) {
        fprintf(stderr, "Invalid mode: %s. Use 'run' or 'compile'.\n", argv[1]);
        return EXIT_FAILURE;
    }

    // run mode
    if(strcmp(argv[1], "run") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Source file required for run mode.\n");
            return EXIT_FAILURE;
        }
        return run_mode(argv[2]);
    }

    // compile mode
    else if(strcmp(argv[1], "compile") == 0) {
        if(argc < 3) {
            fprintf(stderr, "Source file required for compile mode.\n");
            return EXIT_FAILURE;
        }
        return compile_mode(argv[2]);
    }
    else {
        fprintf(stderr, "Unknown mode: %s. Use 'run' or 'compile'.\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    return repl_mode();
}

int repl_mode()
{
    while(1){
        wchar_t input[128];

        printf(">> ");
        if(fgetws(input, sizeof(input)/sizeof(wchar_t), stdin) == NULL) {
            if(feof(stdin)) {
                printf("\nExiting REPL mode.\n");
                break;
            } else {
                perror("Error reading input");
                continue;
            }
        }

        // Удаление символа новой строки
        size_t len = wcslen(input);
        if(len > 0 && input[len - 1] == L'\n') {
            input[len - 1] = L'\0';
        }

        if(input[0] == '\n') continue;
        else if(wcscmp(input, L":quit") == 0) break;
        else if(wcscmp(input, L":reset") == 0) {
            // TODO: implement reset functionality
        }
        else if(wcscmp(input, L":run") == 0) {
            // TODO: implement running the interpreter
        }
        else if(wcscmp(input, L":compile") == 0) {
            // TODO: implement compiling the source
        }
        else if(wcscmp(input, L":help") == 0) {
            printf("Available commands:\n");
            printf("  :quit - Exit REPL mode\n");
            printf("  :help - Show this help message\n");
            printf("  :reset - Reset the REPL state (not implemented yet)\n");
            printf("  :run - Run the interpreter (not implemented yet)\n");
            printf("  :compile - Compile the source (not implemented yet)\n");
        }
        else {
            // TODO: implement executing
            printf("%ls\n", input);
        }
    }
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
