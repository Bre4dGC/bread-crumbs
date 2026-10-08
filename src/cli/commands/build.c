#include <stdio.h>

#include "cli/commands.h"

static int build_handler(int argc, char** argv, void* userdata)
{
    (void)argc;
    (void)argv;
    (void)userdata;
    printf("TODO: build: building project\n");
    return 0;
}

void register_build_command(command_list_t* list)
{
    cmd_add(list, "build", "Build the project", "[<options>]", new_option_list(), build_handler, NULL);
}
