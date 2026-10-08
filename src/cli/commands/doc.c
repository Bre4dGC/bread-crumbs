#include <stdio.h>

#include "cli/commands.h"

static int doc_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: doc: missing name\n");
        fprintf(stderr, "Usage: crum doc <name>\n");
        return 1;
    }
    printf("TODO: doc: showing documentation for '%s'\n", argv[0]);
    return 0;
}

void register_doc_command(command_list_t* list)
{
    cmd_add(list, "doc", "Show documentation for a command/language/etc", "<name>", new_option_list(), doc_handler, NULL);
}
