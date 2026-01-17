#pragma once
#include <stddef.h>
#include <string.h>

#include "cli/args.h"

typedef int (*command_handler_t)(int argc, char** argv, void* userdata);

typedef struct {
    char* name;
    char* description;
    char* usage;
    cli_option_set_t options;
    command_handler_t handler;
    void* userdata;
} command_t;

typedef struct {
    command_t* items;
    size_t count;
} command_list_t;

command_t* new_command(
    char* name,
    char* description,
    char* usage,
    cli_option_set_t options,
    command_handler_t handler,
    void* userdata
);
void free_command(command_t* cmd);
