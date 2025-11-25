#include <stdio.h>
#include <sys/time.h>

#include "compiler/frontend/parser.h"
#include "common/utils.h"
#include "common/file_reader.h"
#include "common/benchmark.h"

char* filepath;

static int test_count = 0;
static int test_passed = 0;

void run_test(const char* test_name, const char* input, bool should_succeed)
{
    test_count++;
    printf("Running test: %s\n", test_name);
    printf("Input: %s\n", input);
    
    lexer_t *lex = new_lexer(input);
    if(!lex){
        printf("FAIL: Failed to create lexer\n\n");
        return;
    }

    parser_t *pars = new_parser(lex);
    if(!pars){
        printf("FAIL: Failed to create parser\n\n");
        free_lexer(lex);
        return;
    }

    astnode_t* ast = parse_program(pars);
    
    if(should_succeed){
        if(ast){
            printf("PASS: Successfully parsed\n");
            test_passed++;
            // compile_ast(ast, NULL);
            free_ast(ast);
        }
        else{
            printf("FAIL: Expected success but parsing failed\n");
            for(size_t i = 0; i < lex->errors_count; ++i) print_report(lex->errors[i]);
            for(size_t i = 0; i < pars->errors_count; ++i) print_report(pars->errors[i]);
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
    free(filepath);
    printf("\n");
}

int main(void)
{
    bench_start();

    run_test("Function Declaration", "func main() : int { return 0; }", true);
    run_test("Variable Declaration", "var x: int = 42", true);
    run_test("Array Literal", "[1, 2, 3, 4]", true);
    run_test("Function Call", "print(\"hello\")", true);
    run_test("If Statement", "if (x > 0) { return x; }", true);
    run_test("While Loop", "while (true) { break; }", true);
    run_test("For Loop", "for (var i = 0; i < 10; i++) { print(i); }", true);
    run_test("Struct Declaration", "struct Point { var x: int, var y: int }", true);
    run_test("Invalid Syntax", "func ( { }", false);
    run_test("If-Elif-Else Statement", "if (x > 0) { return x; } elif (x < 0) { return -x; } else { return 0; }", true);
    run_test("Union Declaration", "union Color { Red, Green, Blue }", true);

    /* FIX: The match and trait statements is not work yet */
    // run_test("Match Statement", "match (x) { 1 => print(\"one\"), 2 => print(\"two\"), _ => print(\"other\") }", true);
    // run_test("Trait Declaration", "trait Printable { fn print() }", true);

    bench_stop();
    bench_print();
    
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", test_count);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_count - test_passed);
    
    if(test_passed == test_count){
        printf("All tests passed!\n");
        return EXIT_SUCCESS;
    }
    else{
        printf("Some tests failed.\n");
        return EXIT_FAILURE;
    }
}
