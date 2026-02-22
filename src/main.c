#include "cli/core.h"

int main(int argc, char** argv)
{
    cli_t* cli = new_cli(argc, argv);
    int result = cli_run(cli);
    free_cli(cli);
    return result;
}
