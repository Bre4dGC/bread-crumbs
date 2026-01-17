#include <stdlib.h>
#include <string.h>

#include "cli/core.h"
#include "cli/commands.h"

cli_t* new_cli(int argc, char **argv)
{
    cli_t* cli = (cli_t*)malloc(sizeof(cli_t));
    if(!cli) return NULL;
    cli->argc = argc;
    cli->argv = argv;
    cli->commands.items = NULL;
    cli->commands.count = 0;
    return cli;
}

command_t find_command(cli_t* cli, const char* name)
{
    if(!cli || !name) return (command_t){0};
    
    for(size_t i = 0; i < cli->commands.count; ++i){
        if(cli->commands.items[i].name && strcmp(name, cli->commands.items[i].name) == 0){
            return cli->commands.items[i];
        }
    }
    return (command_t){0};
}

void free_cli(cli_t* cli)
{
    if(!cli) return;
    
    if(cli->commands.items){
        for(size_t i = 0; i < cli->commands.count; ++i){
            free_command(&cli->commands.items[i]);
        }
        free(cli->commands.items);
    }
    free(cli);
}
