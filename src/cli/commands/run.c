#include <stdio.h>

#include "cli/commands.h"

static int run_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: run: missing source file\n");
        fprintf(stderr, "Usage: crum run <source> [<options>]\n");
        return 1;
    }
    printf("TODO: run: executing '%s'\n", argv[0]);
    return 0;
}

void register_run_command(command_list_t* list)
{
    cmd_add(list, "run", "Run without build (hot-reload)", "[<source>] [<options>]", new_option_list(), run_handler, NULL);
}
