#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/diagnostic/errors.h"
#include "common/utils.h"

char* filepath;

int main(int argc, char** argv)
{
    (void)argc; // Unused parameter
    filepath = util_strdup(argv[0]);

    struct error* err = new_error(
        SEVERITY_ERROR,
        ERROR_TYPE_LEXER,
        LEXER_ERROR_UNCLOSED_STRING,
        10,
        23,
        1,
        "var name: str = \"bread");
    
    if(err){
        print_error(err);
        free_error(err);
        free(filepath);
    }
    else {
        printf("Failed to create error\n");
    }

    return EXIT_SUCCESS;
}
