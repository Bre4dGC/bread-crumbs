#include "parser.h"

int main(void)
{
    const char *input = "{ return x }";
    
    struct lexer *lexer = new_lexer(input);
    struct parser *parser = new_parser(lexer);
    struct ast_node *ast = parse_expr(parser);

    free_parser(parser);

    return 0;
}