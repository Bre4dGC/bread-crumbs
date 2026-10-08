#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/ds/strings.h"
#include "cli/args.h"

typedef int (*command_handler_t)(int argc, char** argv, void* userdata);

typedef struct {
    string_t name;
    string_t description;
    string_t usage;
    cli_option_set_t options;
    command_handler_t handler;
    void* userdata;
} command_t;

typedef struct {
    command_t* items;
    size_t count;
} command_list_t;

command_list_t new_command_list(void);
void free_command_list(command_list_t* list);

void cmd_add(
    command_list_t* list,
    const char* name,
    const char* description,
    const char* usage,
    cli_option_set_t options,
    command_handler_t handler,
    void* userdata
);
