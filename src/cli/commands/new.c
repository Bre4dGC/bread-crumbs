#include <stdio.h>

#include "cli/commands.h"

static int new_handler(int argc, char** argv, void* userdata)
{
    (void)userdata;
    if(argc < 1){
        fprintf(stderr, "TODO: new: missing project name\n");
        fprintf(stderr, "Usage: crum new <name> [<template>]\n");
        return 1;
    }
    if(argc > 1){
        printf("TODO: new: creating project '%s' from template '%s'\n", argv[0], argv[1]);
    }
    else {
        printf("TODO: new: creating project '%s'\n", argv[0]);
    }
    return 0;
}

void register_new_command(command_list_t* list)
{
    cmd_add(list, "new", "Create a new project", "<name> [<template>]", new_option_list(), new_handler, NULL);
}
