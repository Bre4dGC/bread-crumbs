#include <stdio.h>
#include "parser.h"

int main(void)
{
    const char *input = "if(true){ return x }";
    
    struct lexer *lex = new_lexer(input);
    if(!lex) {
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    struct parser *pars = new_parser(lex);
    if(!pars) {
        fprintf(stderr, "Failed to create parser\n");
        free_lexer(lex);
        return 1;
    }

    struct ast_node* ast = parse_expr(pars);
    if(!ast) {
        fprintf(stderr, "Failed to parse expression\n");
        for(int i = 0; i < pars->errors_count; ++i){
            print_error(pars->errors[i]);
        }
        free_parser(pars);
        return 1;
    }

    // Successfully parsed the expression
    printf("Parsed expression successfully.\n");
    free_ast(ast);
    free_parser(pars);

    return 0;
}