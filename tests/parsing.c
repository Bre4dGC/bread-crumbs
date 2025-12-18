#include "compiler/frontend/parser.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();

    lexer_t *lexer = new_lexer("func test() { var x : int = 10; if(x >= 10) { x += 1; } else { x -= 1; } return x; }");
    if(!lexer){
        printf("FAIL: Failed to create lexer\n\n");
        return 1;
    }

    parser_t *parser = new_parser(lexer);
    if(!parser){
        printf("FAIL: Failed to create parser\n\n");
        free_lexer(lexer);
        return 1;
    }

    astnode_t* ast = parse_program(parser);

    if(ast){
        printf("PASS: Successfully parsed\n");
        free_ast(ast);
    }
    else{
        printf("FAIL: Expected success but parsing failed\n");
        for(size_t i = 0; i < lexer->errors_count; ++i) print_report(lexer->errors[i]);
        for(size_t i = 0; i < parser->errors_count; ++i) print_report(parser->errors[i]);
    }

    free_parser(parser);
    
    bench_print("Test parser");
    bench_stop();
    return 0;
}
