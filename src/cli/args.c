#include <stdlib.h>     // malloc, free

#include "cli/args.h"   // cli_option_set_t

cli_option_set_t new_option_list(void)
{
    return (cli_option_set_t){
        .items = NULL,
        .count = 0
    };
}

void free_option_list(cli_option_set_t* opt_set)
{
    if(!opt_set) return;

    if(opt_set->items){
        for(size_t i = 0; i < opt_set->count; ++i){
            if(opt_set->items[i].name)          free(opt_set->items[i].name);
            if(opt_set->items[i].description)   free(opt_set->items[i].description);
            if(opt_set->items[i].value_name)    free(opt_set->items[i].value_name);
            if(opt_set->items[i].default_value) free(opt_set->items[i].default_value);
        }
        free(opt_set->items);
        opt_set->items = NULL;
    }
    opt_set->count = 0;
}
