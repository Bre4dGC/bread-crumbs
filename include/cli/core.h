#pragma once

#include "core/arena.h"
#include "cli/commands.h"

typedef struct {
    int argc;
    char** argv;
    command_list_t commands;
} cli_t;

cli_t* new_cli(int argc, char** argv);
void free_cli(cli_t* cli);
int cli_run(cli_t* cli);
