#include <stdio.h>

#include "cli/commands.h"

static int install_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: install: missing package name\n");
        fprintf(stderr, "Usage: crum install <name> [<options>]\n");
        return 1;
    }
    printf("TODO: install: installing package '%s'\n", argv[0]);
    return 0;
}

void register_install_command(command_list_t* list)
{
    cmd_add(list, "install", "Install a package", "<name> [<options>]", new_option_list(), install_handler, NULL);
}
