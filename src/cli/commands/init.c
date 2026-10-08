#include <stdio.h>

#include "cli/commands.h"

static int init_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc > 0){
        printf("TODO: init: initializing project from template '%s'\n", argv[0]);
    }
    else {
        printf("TODO: init: initializing project\n");
    }
    return 0;
}

void register_init_command(command_list_t* list)
{
    cmd_add(list, "init", "Initialize a new project", "[<template>]", new_option_list(), init_handler, NULL);
}
