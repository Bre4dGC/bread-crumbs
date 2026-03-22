#pragma once

#include <stddef.h>     // size_t
#include <stdbool.h>    // bool

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

cli_option_set_t new_option_list(void);
void free_option_list(cli_option_set_t* opt_set);
