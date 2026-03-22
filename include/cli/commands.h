#pragma once

#include <stddef.h>     // size_t

#include "core/ds/strings.h"    // string_t
#include "cli/args.h"   // cli_option_set_t

typedef int (*command_handler_t)(int argc, char** argv, void* userdata);

typedef struct {
    string_t name;
    const char* description;
    const char* usage;
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
command_list_t new_command_list(void);
void free_command_list(command_list_t* list);
