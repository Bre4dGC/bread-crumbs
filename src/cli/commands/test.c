#include <stdio.h>

#include "cli/commands.h"

static int test_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        printf("TODO: test: running all tests\n");
        return 0;
    }
    printf("TODO: test: running tests for '%s'\n", argv[0]);
    return 0;
}

void register_test_command(command_list_t* list)
{
    cmd_add(list, "test", "Run tests", "[<source>] [<options>]", new_option_list(), test_handler, NULL);
}
