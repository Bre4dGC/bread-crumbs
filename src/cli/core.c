#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/ds/hashmap.h"
#include "cli/core.h"
#include "cli/commands.h"

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
    if(!cli) return;
    free_command_list(&cli->commands);
    free_option_list(&cli->options);
}

command_t* find_command(cli_t* cli, const char* name)
{
    if(!cli || !name) return NULL;

    for (size_t i = 0; i < cli->commands.count; ++i){
        if(cli->commands.items[i].name.hash == hm_hash(name)){
            return &cli->commands.items[i];
        }
    }
    return NULL;
}

int cli_run(cli_t* cli)
{
    if(!cli) return 1;

    int show_help = 0;
    int show_version = 0;
    int cmd_index = -1;

    for (int i = 1; i < cli->argc; i++){
        if(strcmp(cli->argv[i], "--help") == 0 || strcmp(cli->argv[i], "-h") == 0){
            show_help = 1;
        }
        else if(strcmp(cli->argv[i], "--version") == 0 || strcmp(cli->argv[i], "-V") == 0){
            show_version = 1;
        }
        else if(strcmp(cli->argv[i], "--quiet") == 0 || strcmp(cli->argv[i], "-q") == 0){
            printf("TODO: quiet mode enabled\n");
        }
        else if(strcmp(cli->argv[i], "--verbose") == 0 || strcmp(cli->argv[i], "-v") == 0){
            printf("TODO: verbose mode enabled\n");
        }
        else if(strcmp(cli->argv[i], "--output") == 0 || strcmp(cli->argv[i], "-o") == 0){
            if(i + 1 < cli->argc){
                i++;
                printf("TODO: output file set to '%s'\n", cli->argv[i]);
            }
            else {
                fprintf(stderr, "TODO: --output requires a filename\n");
            }
        }
        else if(cli->argv[i][0] != '-' && cmd_index < 0){
            cmd_index = i;
        }
    }

    if(show_version){
        command_t* ver = find_command(cli, "version");
        if(ver) return ver->handler(0, NULL, ver->userdata);
        return 0;
    }

    if(show_help || cmd_index < 0){
        command_t* help = find_command(cli, "help");
        if(!help) return 0;
        int argc = (cmd_index > 0) ? 1 : 0;
        char* args = (cmd_index > 0) ? cli->argv[cmd_index] : NULL;
        return help->handler(argc, args ? &args : NULL, help->userdata);
    }

    command_t* cmd = find_command(cli, cli->argv[cmd_index]);
    if(!cmd){
        fprintf(stderr, "crum: unknown command '%s'\n", cli->argv[cmd_index]);
        fprintf(stderr, "Run 'crum --help' for usage.\n");
        return 1;
    }

    return cmd->handler(
        cli->argc - cmd_index - 1,
        cli->argv + cmd_index + 1,
        cmd->userdata
    );
}
