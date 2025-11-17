#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser.h"
#include "compiler/frontend/semantic.h"
#include "compiler/core/diagnostic.h"
#include "common/file_reader.h"

char* filepath = NULL;

int repl_mode(void);
int run_file(const char *filepath);

int main(int argc, char *argv[])
{
    if(argc < 2){
        return repl_mode();
    }

    if(strcmp(argv[1], "help") == 0){
        fprintf(stderr, "Usage: %s [options] <file.brc>\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "   (no arguments)   - Start REPL mode\n");
        fprintf(stderr, "   run <file.brc>   - Run the program from .brc file\n");
        fprintf(stderr, "   build <file.brc> - build the project\n");
        fprintf(stderr, "   help             - Show this help message\n");
        return EXIT_SUCCESS;
    }

    if(argc == 2){
        filepath = argv[1];
    }
    else if(argc >= 3 && strcmp(argv[1], "run") == 0){
        filepath = argv[2];
    }
    else {
        fprintf(stderr, "Error: Invalid arguments. Use '%s help' for usage.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    if(!is_correct_extension(filepath)){
        fprintf(stderr, "Error: File must have .brc extension (got: %s)\n", filepath);
        return EXIT_FAILURE;
    }

    if(!is_file_exists(filepath)){
        fprintf(stderr, "Error: File '%s' does not exist or is not readable\n", filepath);
        return EXIT_FAILURE;
    }

    return run_file(filepath);
}

int repl_mode(void)
{
    char input[1024];

    while(1){
        printf(">> ");
        if(fgets(input, sizeof(input), stdin) == NULL){
            if(feof(stdin)){
                printf("\nExiting REPL mode.\n");
                break;
            }
            else {
                perror("Error reading input");
                continue;
            }
        }

        size_t len = strlen(input);
        if(len > 0 && input[len - 1] == '\n') input[len - 1] = '\0';

        if(input[0] == '\0') continue;
        else if(strcmp(input, ":quit") == 0 || strcmp(input, ":q") == 0) break;
        else if(strcmp(input, ":reset")== 0 || strcmp(input, ":r") == 0){
            // TODO
        }
        else if(strcmp(input, ":help") == 0 || strcmp(input, ":h") == 0){
            printf("Available commands:\n");
            printf("    :quit | :q - Exit REPL mode\n");
            printf("    :help | :h - Show this help message\n");
            printf("    :reset | :r - Reset the REPL state (not implemented yet)\n");
        }
        else {
            printf("%s\n", input);
            // Implement
        }
    }
    return 0;
}

int run_file(const char *filepath)
{
    int result = EXIT_FAILURE;
    
    size_t file_size = 0;
    char* source = NULL;
    struct lexer* lexer = NULL;
    struct parser* parser = NULL;
    struct ast_node* ast = NULL;
    // struct semantic_context* semantic = NULL;

    source = read_file(filepath, &file_size);
    if(!source){
        fprintf(stderr, "Error: Failed to read file '%s'\n", filepath);
        goto cleanup;
    }

    lexer = new_lexer(source);
    if(!lexer){
        fprintf(stderr, "Error: Failed to create lexer\n");
        goto cleanup;
    }

    parser = new_parser(lexer);
    if(!parser){
        fprintf(stderr, "Error: Failed to create parser\n");
        goto cleanup;
    }

    ast = parse_program(parser);
    if(!ast){
        fprintf(stderr, "Error: Parsing failed\n");
        for (size_t i = 0; i < parser->errors_count; ++i) print_report(parser->errors[i]);
        goto cleanup;
    }

    // semantic = new_semantic_context();
    // if(!semantic){
    //     fprintf(stderr, "Error: Failed to create semantic context\n");
    //     goto cleanup;
    // }

    // if(!analyze_ast(semantic, ast)){
    //     fprintf(stderr, "Error: Semantic analysis failed\n");
    //     for (size_t i = 0; i < semantic->errors_count; ++i) print_report(semantic->errors[i]);
    //     goto cleanup;
    // }

    // if(!compile_ast(ast, NULL)){
    //     fprintf(stderr, "Error: Compilation failed\n");
    //     goto cleanup;
    // }
    result = EXIT_SUCCESS;

cleanup:
    // if(semantic) free_semantic_context(semantic);
    if(ast) free_ast(ast);
    if(parser) free_parser(parser);
    if(source) free(source);
    
    return result;
}
