#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "errors.h"

char *file_name;

int repl_mode(void);
int run_mode(const char *src);
int compile_mode(const char *src);

int main(int argc, char *argv[])
{
    if(argc < 2) {
        return repl_mode();
    }
    
    file_name = NULL;

    // check for help mode
    if(argc == 2 && strcmp(argv[1], "help") == 0) {
        fprintf(stderr, "Usage: %s <mode> [source]\n", argv[0]);
        fprintf(stderr, "Modes:\n");
        fprintf(stderr, "   run <source>     - Run the interpreter with the source file\n");
        fprintf(stderr, "   compile <source> - Compile the source file with JIT\n");
        fprintf(stderr, "   (no arguments)   - Start REPL mode\n");
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

    // jit mode
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

int repl_mode(void)
{
    char input[1024];

    while (1) {
        printf(">> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nExiting REPL mode.\n");
                break;
            } else {
                perror("Error reading input");
                continue;
            }
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';

        if (input[0] == '\0') continue;
        else if (strcmp(input, ":quit") == 0) break;
        else if (strcmp(input, ":reset") == 0) {
            // TODO
        }
        else if (strcmp(input, ":help") == 0) {
            printf("Available commands:\n");
            printf("    :quit - Exit REPL mode\n");
            printf("    :help - Show this help message\n");
            printf("    :reset - Reset the REPL state (not implemented yet)\n");
        }
        else {
            printf("%s\n", input);
        }
    }
    return 0;
}

char* get_line(const char *src)
{
    if(!src) return NULL;

    size_t length = 0;

    while(src[length] != '\0' && src[length] != '\n') {
        length++;
    }
    
    char *line = (char*)malloc((length + 1) * sizeof(char));
    if(!line) return NULL;

    for(size_t i = 0; i < length; ++i) {
        line[i] = src[i];
    }

    line[length] = L'\0';

    return line;
}

int run_mode(const char *src)
{
    size_t len = strlen(src);
    char *wsrc = (char*)malloc((len + 1) * sizeof(char));
    if(!wsrc) return EXIT_FAILURE;
    for(size_t i = 0; i < len; ++i) wsrc[i] = (unsigned char)src[i];
    wsrc[len] = L'\0';

    char *line = get_line(wsrc);
    if(!line) {
        free(wsrc);
        return EXIT_FAILURE;
    }

    struct lexer *lexer = new_lexer(line);
    if(!lexer){
        free(line);
        free(wsrc);
        return EXIT_FAILURE;
    }
    struct token token;
    do {
        token = next_token(lexer);
        free_token(&token);
    } while(!(token.category == CATEGORY_SERVICE && token.type_service == SERV_EOF));

    free_token(&token);
    free_lexer(lexer);
    free(line);
    free(wsrc);

    return 0;
}

int compile_mode(const char *src)
{
    printf("JIT-compiler not able yet :(");
    return 0;
}
