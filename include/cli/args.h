#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

enum cli_option_kind {
    CLI_OPTION_FLAG,
    CLI_OPTION_VALUE
};

typedef struct {
    char* name;
    char* description;
    enum cli_option_kind kind;
    char* value_name;
    char* default_value;
    bool required;
} cli_option_t;

typedef struct {
    cli_option_t* items;
    size_t count;
} cli_option_set_t;

cli_option_set_t* new_args(void);
void free_args(cli_option_set_t* opt_set);
bool add_arg(cli_option_set_t* opt_set, const char* name, const char* description, enum cli_option_kind kind, const char* value_name, const char* default_value, bool required);
bool parse_args(cli_option_set_t* opt_set, int argc, char** argv);
