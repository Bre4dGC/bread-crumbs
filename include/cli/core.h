// cli/core.h
#pragma once

#include "cli/commands.h"

typedef struct {
    int argc;
    char** argv;
    command_list_t commands;
} cli_t;

cli_t* new_cli(int argc, char** argv);
command_t find_command(cli_t* cli, const char* name);
void free_cli(cli_t* cli);
