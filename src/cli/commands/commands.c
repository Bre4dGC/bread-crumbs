#include <stdlib.h>

#include "cli/commands.h"

command_t* new_command(
    char* name,
    char* description,
    char* usage,
    cli_option_set_t options,
    command_handler_t handler,
    void* userdata)
{
    command_t* cmd = (command_t*)malloc(sizeof(command_t));
    if(!cmd) return NULL;
    cmd->name = name;
    cmd->description = description;
    cmd->usage = usage;
    cmd->options = options;
    cmd->handler = handler;
    cmd->userdata = userdata;
    return cmd;
}

void free_command(command_t* cmd)
{
    if(!cmd) return;

    if(cmd->name)        free(cmd->name);
    if(cmd->description) free(cmd->description);
    if(cmd->usage)       free(cmd->usage);
    free_args(&cmd->options);
    free(cmd);
}
