#include "parser.h"

int main(void)
{
    const wchar_t *input = L"if (x > 0) { return x; }";
    
    Lexer *lexer = lex_new(input);
    Parser *parser = parser_new(lexer);
    ASTNode *ast = parse_expr(parser);

    parser_free(parser);

    return 0;
}