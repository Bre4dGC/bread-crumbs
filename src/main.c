#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/errors.h"
#include "utils/file_reader.h"

char *file_name;

int repl_mode(void);
int run_file(const char *filepath);

int main(int argc, char *argv[])
{
    // No arguments - start REPL
    if(argc < 2) {
        return repl_mode();
    }
    
    file_name = NULL;

    // Check for help mode
    if(strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        fprintf(stderr, "Usage: %s [options] <file.brc>\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   <file.brc>     - Run the program from .brc file\n");
        fprintf(stderr, "   (no arguments) - Start REPL mode\n");
        fprintf(stderr, "   help, --help   - Show this help message\n");
        return EXIT_SUCCESS;
    }

    // Determine filepath (handle both "prog file.brc" and "prog run file.brc")
    const char* filepath = NULL;
    if (argc == 2) {
        filepath = argv[1];
    } else if (argc >= 3 && strcmp(argv[1], "run") == 0) {
        filepath = argv[2];
    } else {
        fprintf(stderr, "Error: Invalid arguments. Use '%s help' for usage.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Check if file has .brc extension
    if (!is_brc_file(filepath)) {
        fprintf(stderr, "Error: File must have .brc extension (got: %s)\n", filepath);
        return EXIT_FAILURE;
    }

    // Check if file exists
    if (!file_exists(filepath)) {
        fprintf(stderr, "Error: File '%s' does not exist or is not readable\n", filepath);
        return EXIT_FAILURE;
    }

    file_name = (char*)filepath;
    return run_file(filepath);
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
            }
            else {
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

int run_file(const char *filepath)
{
    // Read entire file into memory
    size_t file_size = 0;
    char* source = read_file(filepath, &file_size);
    if (!source) {
        return EXIT_FAILURE;
    }

    printf("Read %zu bytes from '%s'\n", file_size, filepath);

    // Create lexer with file contents
    struct lexer* lexer = new_lexer(source);
    if (!lexer) {
        fprintf(stderr, "Error: Failed to create lexer\n");
        free(source);
        return EXIT_FAILURE;
    }

    struct lexer *lex = new_lexer(source);
    if(!lex){
        printf("FAIL: Failed to create lexer\n\n");
        return EXIT_FAILURE;
    }

    struct parser *pars = new_parser(lex);
    if(!pars){
        printf("FAIL: Failed to create parser\n\n");
        free_lexer(lex);
        return EXIT_FAILURE;
    }

    struct ast_node* ast = parse_expr(pars);

    if(ast){
        compile_ast(ast, NULL);
        free_ast(ast);
    }
    else{
        for(size_t i = 0; i < pars->errors_count; ++i){
            print_error(pars->errors[i]);
        }
    }
    
    free_parser(pars);
    free(source);

    return EXIT_SUCCESS;
}
