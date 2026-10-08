#include <stdio.h>

#include "cli/commands.h"

static int remove_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: remove: missing dependency name\n");
        fprintf(stderr, "Usage: crum remove <name>\n");
        return 1;
    }
    printf("TODO: remove: removing dependency '%s'\n", argv[0]);
    return 0;
}

void register_remove_command(command_list_t* list)
{
    cmd_add(list, "remove", "Remove a dependency", "<name>", new_option_list(), remove_handler, NULL);
}
