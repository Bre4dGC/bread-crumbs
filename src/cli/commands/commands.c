#include <stdlib.h>     // malloc, free
#include <string.h>

#include "cli/commands.h"   // command_t, cli_option_set_t
#include "core/ds/hashmap.h"// hm_hash

command_list_t new_command_list(void)
{
    return (command_list_t){.items = NULL, .count = 0};
}

void free_command_list(command_list_t* list)
{
    if(!list) return;
    if(list->items){
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
}

void cmd_add(
    command_list_t* list,
    const char* name,
    const char* description,
    const char* usage,
    cli_option_set_t options,
    command_handler_t handler,
    void* userdata)
{
    command_t cmd = {
        .name = {name, 0, hm_hash(name)},
        .description = description,
        .usage = usage,
        .options = options,
        .handler = handler,
        .userdata = userdata
    };

    list->items = realloc(list->items, sizeof(command_t) * (list->count + 1));
    list->items[list->count] = cmd;
    list->count++;
}
