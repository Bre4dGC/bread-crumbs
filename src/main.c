#include <stdlib.h>

#include "cli/core.h"
#include "cli/register.h"

int main(int argc, char** argv)
{
    cli_t cli = new_cli(argc, argv);
    register_commands(&cli);
    int result = cli_run(&cli);
    free_cli(&cli);
    return result;
}
