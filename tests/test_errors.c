#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

int main(void) {
    // Create a new error
    Error *error = error_new(TYPE_ERROR, ERROR_TYPE_LEXER, LEXER_ERROR_UNCLOSED_STRING, 10, 5, L"test_file.c", L"Unclosed string literal");
    
    // Print the error
    if (error) {
        error_print(error);
        error_free(error);
    } else {
        wprintf(L"Failed to create error\n");
    }

    return 0;
}