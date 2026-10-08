#include <stdio.h>

#include "cli/commands.h"

static int debug_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        printf("TODO: debug: starting debugger (no source)\n");
        return 0;
    }
    printf("TODO: debug: debugging '%s'\n", argv[0]);
    return 0;
}

void register_debug_command(command_list_t* list)
{
    cmd_add(list, "debug", "Run a debugger", "[<source>] [<options>]", new_option_list(), debug_handler, NULL);
}
