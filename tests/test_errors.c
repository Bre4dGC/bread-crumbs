#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

int main(void) {
    struct error* err = new_error(
        ERROR_SEVERITY_TYPE,
        ERROR_TYPE_LEXER,
        LEXER_ERROR_UNCLOSED_STRING,
        10,
        23,
        1,
        L"var name: str = \"bread");
    
    if(err){
        print_error(err);
        free_error(err);
    }
    else {
        wprintf(L"Failed to create error\n");
    }

    return 0;
}