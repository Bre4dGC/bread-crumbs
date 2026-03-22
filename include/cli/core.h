#pragma once

#include "cli/args.h"       // cli_option_set_t
#include "cli/commands.h"   // command_list_t, command_t

typedef struct {
    int argc;
    char** argv;
    command_list_t commands;
    cli_option_set_t options;
} cli_t;

cli_t new_cli(int argc, char** argv);
void free_cli(cli_t* cli);

command_t find_command(cli_t* cli, const char* name);
