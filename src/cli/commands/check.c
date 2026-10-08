#include <stdio.h>

#include "cli/commands.h"

static int check_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: check: missing source file\n");
        fprintf(stderr, "Usage: crum check <source>\n");
        return 1;
    }
    printf("TODO: check: analyzing '%s'\n", argv[0]);
    return 0;
}

void register_check_command(command_list_t* list)
{
    cmd_add(list, "check", "Check errors, anti-patterns, etc", "[<source>]", new_option_list(), check_handler, NULL);
}
