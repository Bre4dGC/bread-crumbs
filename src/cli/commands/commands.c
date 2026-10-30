#include <stdlib.h>
#include <string.h>

#include "cli/commands.h"
#include "core/ds/hashmap.h"

command_list_t new_command_list(void)
{
    return (command_list_t){.items = NULL, .count = 0};
}

void free_command_list(command_list_t* list)
{
    if(!list) return;
    if(list->items){
        for (size_t i = 0; i < list->count; i++){
            free_option_list(&list->items[i].options);
        }
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
}

static string_t string_view(const char* str)
{
    if(!str) return (string_t){0};
    return (string_t){str, strlen(str), hm_hash(str)};
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
        .name = string_view(name),
        .description = string_view(description),
        .usage = string_view(usage),
        .options = options,
        .handler = handler,
        .userdata = userdata
    };

    command_t* items = realloc(list->items, sizeof(command_t) * (list->count + 1));
    if(!items) return;

    list->items = items;
    list->items[list->count] = cmd;
    list->count++;
}
