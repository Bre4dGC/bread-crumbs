#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "core/ds/strings.h"

enum cli_option_kind {
    CLI_OPTION_FLAG,
    CLI_OPTION_VALUE
};

typedef struct {
    string_t name;
    string_t description;
    enum cli_option_kind kind;
    string_t value_name;
    string_t default_value;
    bool required;
} cli_option_t;

typedef struct {
    cli_option_t* items;
    string_pool_t* pool;
    size_t count;
} cli_option_set_t;

cli_option_set_t new_option_list(void);
void free_option_list(cli_option_set_t* opt_set);

void option_add(
    cli_option_set_t* set,
    const char* name,
    const char* description,
    enum cli_option_kind kind,
    const char* value_name,
    const char* default_value,
    bool required
);
