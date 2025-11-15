#include "compiler/middle/semantic.h"
#include "common/utils.h"

char* filepath;

int main(int argc, char** argv)
{
    (void)argc;
    filepath = util_strdup(argv[0]);
    free(filepath);
    return EXIT_SUCCESS;
}
