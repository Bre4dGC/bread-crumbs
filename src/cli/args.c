#include "cli/args.h"

cli_option_set_t* new_args(void)
{
    cli_option_set_t* opt_set = malloc(sizeof(cli_option_set_t));
    if(!opt_set) return NULL;
    opt_set->items = NULL;
    opt_set->count = 0;
    return opt_set;
}

void free_args(cli_option_set_t* opt_set)
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

bool add_arg(cli_option_set_t* opt_set, const char* name, const char* description, enum cli_option_kind kind, const char* value_name, const char* default_value, bool required)
{
    // TODO: implement
}

bool parse_args(cli_option_set_t* opt_set, int argc, char** argv)
{
    // TODO: implement
}
