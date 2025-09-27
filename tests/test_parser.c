#include <stdio.h>
#include "parser.h"

#ifdef DEBUG
#include "debug.h"
#endif

static int test_count = 0;
static int test_passed = 0;

void run_test(const char* test_name, const char* input, bool should_succeed)
{
    test_count++;
    printf("Running test: %s\n", test_name);
    printf("Input: %s\n", input);
    
    struct lexer *lex = new_lexer(input);
    if(!lex){
        printf("FAIL: Failed to create lexer\n\n");
        return;
    }

    struct parser *pars = new_parser(lex);
    if(!pars){
        printf("FAIL: Failed to create parser\n\n");
        free_lexer(lex);
        return;
    }

    struct ast_node* ast = parse_expr(pars);
    
    if(should_succeed){
        if(ast){
            printf("PASS: Successfully parsed\n");
            test_passed++;
            #ifdef DEBUG
            compile_ast(ast, NULL);
            #endif
            free_ast(ast);
        }
        else{
            printf("FAIL: Expected success but parsing failed\n");
            for(size_t i = 0; i < pars->errors_count; ++i){
                print_error(pars->errors[i]);
            }
        }
    }
    else{
        if(!ast){
            printf("PASS: Expected failure and parsing failed as expected\n");
            test_passed++;
        }
        else{
            printf("FAIL: Expected failure but parsing succeeded\n");
            free_ast(ast);
        }
    }
    
    free_parser(pars);
    printf("\n");
}

int main(void)
{
    printf("=== Parser Test Suite ===\n\n");
    
    run_test("Function Declaration", "func main() => int { return 0; }", true);
    run_test("Variable Declaration", "var x: int = 42", true);
    run_test("Array Literal", "[1, 2, 3, 4]", true);
    run_test("Function Call", "print(\"hello\")", true);
    run_test("If Statement", "if (x > 0) { return x; }", true);
    run_test("While Loop", "while (true) { break; }", true);
    run_test("For Loop", "for (var i = 0; i < 10; i++) { print(i); }", true);
    run_test("Struct Declaration", "struct Point { x: int, y: int }", true);
    run_test("Invalid Syntax", "func ( { }", false);
    run_test("If-Elif-Else Statement", "if (x > 0) { return x; } elif (x < 0) { return -x; } else { return 0; }", true);
    run_test("Union Declaration", "union Color { Red, Green, Blue }", true);

    /* FIX: The match and trait statements is not work yet */
    run_test("Match Statement", "match (x) { 1 => println(\"one\"), 2 => println(\"two\"), _ => println(\"other\") }", true);
    // run_test("Trait Declaration", "trait Printable { fn print() }", true);
    
    printf("=== Test Results ===\n");
    printf("Tests run: %d\n", test_count);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_count - test_passed);
    
    if(test_passed == test_count){
        printf("All tests passed!\n");
        return 0;
    }
    else{
        printf("Some tests failed.\n");
        return 1;
    }
}
