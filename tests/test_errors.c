#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/diagnostic.h"
#include "common/utils.h"

char* filepath;

int main()
{
    filepath = util_strdup("file_with_errors.brc");

    struct report* err = new_report(
        SEVERITY_ERROR, ERROR_UNCLOSED_STRING, 10, 23, 1, "var name: str = \"bread"
    );
    
    if(err){
        print_report(err);
        free_report(err);
        free(filepath);
    }
    else {
        printf("Failed to create report\n");
    }

    return EXIT_SUCCESS;
}
