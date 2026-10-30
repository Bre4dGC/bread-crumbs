#include <stdlib.h>
#include <string.h>

#include "cli/args.h"
#include "core/ds/strings.h"

cli_option_set_t new_option_list(void)
{
    cli_option_set_t set = {
        .items = NULL,
        .pool = NULL,
        .count = 0
    };

    set.pool = malloc(sizeof(*set.pool));
    if(set.pool) *set.pool = new_string_pool(SP_DEF_CAPACITY);

    return set;
}

void free_option_list(cli_option_set_t* opt_set)
{
    if(!opt_set) return;

    if(opt_set->pool){
        free_string_pool(opt_set->pool);
        free(opt_set->pool);
        opt_set->pool = NULL;
    }

    free(opt_set->items);
    opt_set->items = NULL;
    opt_set->count = 0;
}

void option_add(
    cli_option_set_t* set,
    const char* name,
    const char* description,
    enum cli_option_kind kind,
    const char* value_name,
    const char* default_value,
    bool required)
{
    cli_option_t* new_items = realloc(set->items, sizeof(cli_option_t) * (set->count + 1));
    if(!new_items) return;

    set->items = new_items;
    set->items[set->count] = (cli_option_t){
        .name = new_string(set->pool, name),
        .description = new_string(set->pool, description),
        .kind = kind,
        .value_name = new_string(set->pool, value_name),
        .default_value = new_string(set->pool, default_value),
        .required = required
    };
    set->count++;
}
