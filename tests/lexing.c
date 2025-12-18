#include "compiler/frontend/lexer.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();
    
    lexer_t* lexer = new_lexer("if(x > 10) { print(\"x is greater than 10\"); } elif(x < 5) { print(\"x is less than 5\"); } else { print(\"x is between 5 and 10\"); }");

    if(!lexer){
        fprintf(stderr, "Failed to create lexer\n");
        return 1;
    }

    token_t token;
    while((token = next_token(lexer)).category != CATEGORY_SERVICE || token.type_service != SERV_EOF){
        free_token(&token);
    }
    
    free_token(&token);
    free_lexer(lexer);
    
    bench_print("Test lexer");
    bench_stop();
    return 0;
}
