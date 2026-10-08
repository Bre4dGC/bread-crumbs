#include <stdio.h>

#include "cli/commands.h"

static int fmt_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: fmt: missing source file\n");
        fprintf(stderr, "Usage: crum fmt <source>\n");
        return 1;
    }
    printf("TODO: fmt: formatting '%s'\n", argv[0]);
    return 0;
}

void register_fmt_command(command_list_t* list)
{
    cmd_add(list, "fmt", "Format code", "[<source>]", new_option_list(), fmt_handler, NULL);
}
