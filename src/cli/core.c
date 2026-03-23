#include <stdlib.h>     // malloc, free
#include <string.h>     // strcmp

#include "core/ds/hashmap.h"// hs_hash
#include "cli/core.h"       // cli_t, command_t
#include "cli/commands.h"   // free_command

cli_t new_cli(int argc, char **argv)
{
    return (cli_t){
        .argc = argc,
        .argv = argv,
        .commands = new_command_list(),
        .options = new_option_list()
    };
}

void free_cli(cli_t* cli)
{
    free_command_list(&cli->commands);
    free_option_list(&cli->options);
    free(cli);
}

command_t find_command(cli_t* cli, const char* name)
{
    if(!cli || !name) return (command_t){0};

    for(size_t i = 0; i < cli->commands.count; ++i){
        if(cli->commands.items[i].name.hash == hm_hash(name)){
            return cli->commands.items[i];
        }
    }
    return (command_t){0};
}
