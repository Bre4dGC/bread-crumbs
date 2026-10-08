#include <stdio.h>
#include <string.h>

#include "cli/core.h"
#include "cli/commands.h"
#include "cli/register.h"

void register_run_command(command_list_t* list);
void register_build_command(command_list_t* list);
void register_check_command(command_list_t* list);
void register_repl_command(command_list_t* list);
void register_doc_command(command_list_t* list);
void register_new_command(command_list_t* list);
void register_init_command(command_list_t* list);
void register_debug_command(command_list_t* list);
void register_test_command(command_list_t* list);
void register_fmt_command(command_list_t* list);
void register_install_command(command_list_t* list);
void register_add_command(command_list_t* list);
void register_remove_command(command_list_t* list);

static int help_handler(int argc, char** argv, void* userdata);
static int version_handler(int argc, char** argv, void* userdata);

void register_commands(cli_t* cli)
{
    register_run_command(&cli->commands);
    register_build_command(&cli->commands);
    register_check_command(&cli->commands);
    register_repl_command(&cli->commands);
    register_doc_command(&cli->commands);
    register_new_command(&cli->commands);
    register_init_command(&cli->commands);
    register_debug_command(&cli->commands);
    register_test_command(&cli->commands);
    register_fmt_command(&cli->commands);
    register_install_command(&cli->commands);
    register_add_command(&cli->commands);
    register_remove_command(&cli->commands);

    cmd_add(&cli->commands, "help", "Show help message", "[<command>]", new_option_list(), help_handler, cli);

    cmd_add(&cli->commands, "version", "Show version information", "", new_option_list(), version_handler, NULL);
}

static int help_handler(int argc, char** argv, void* userdata)
{
    cli_t* cli = (cli_t*)userdata;
    if(!cli) return 1;

    if(argc > 0 && argv[0]){
        command_t* cmd = find_command(cli, argv[0]);
        if(!cmd){
            fprintf(stderr, "crum: unknown command '%s'\n", argv[0]);
            return 1;
        }
        printf("Usage: crum %s %s\n", cmd->name.data, cmd->usage.data ? cmd->usage.data : "");
        if(cmd->description.data) printf("\n%s\n", cmd->description.data);
        return 0;
    }

    printf("Usage: crum <command> [<args>]\n\n");
    printf("Commands:\n");
    for (size_t i = 0; i < cli->commands.count; i++){
        printf("  %-12s %s\n", cli->commands.items[i].name.data, cli->commands.items[i].description.data ? cli->commands.items[i].description.data : "");
    }
    printf("\nOptions:\n");
    printf("  --help, -h       Show this help message\n");
    printf("  --version, -V    Show version information\n");
    printf("  --quiet, -q      Quiet mode\n");
    printf("  --verbose, -v    Verbose mode\n");
    printf("  --output, -o     Output file\n");
    return 0;
}

static int version_handler(int argc, char** argv, void* userdata)
{
    (void)argc;
    (void)argv;
    (void)userdata;
    printf("%s version %s\n", CLI_NAME, CLI_VERSION);
    return 0;
}
