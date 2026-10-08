#include <stdio.h>

#include "cli/commands.h"

static int repl_handler(int argc, char** argv, void* userdata)
{
    (void)argc;
    (void)argv;
    (void)userdata;
    printf("TODO: repl: starting interactive session\n");
    return 0;
}

void register_repl_command(command_list_t* list)
{
    cmd_add(list, "repl", "Start an interactive REPL session", "", new_option_list(), repl_handler, NULL);
}
