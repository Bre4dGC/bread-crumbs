#include "compiler/frontend/semantic.h"
#include "common/utils.h"

char* filepath;

int main(void)
{
    filepath = util_strdup("file_with_errors.brc");
    free(filepath);
    return EXIT_SUCCESS;
}
