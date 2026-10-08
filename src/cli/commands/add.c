#include <stdio.h>

#include "cli/commands.h"

static int add_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: add: missing dependency name\n");
        fprintf(stderr, "Usage: crum add <name> [<options>]\n");
        return 1;
    }
    printf("TODO: add: adding dependency '%s'\n", argv[0]);
    return 0;
}

void register_add_command(command_list_t* list)
{
    cmd_add(list, "add", "Add a dependency", "<name> [<options>]", new_option_list(), add_handler, NULL);
}
