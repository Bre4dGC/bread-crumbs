#include <stdio.h>
#include <string.h>

#include "compiler/core/string_pool.h"

int main(void)
{
    string_pool_t pool = new_string_pool(ARENA_DEFAULT_SIZE);
    char* string = new_string(&pool, "Hello, World!");
    if(!string){
        printf("Failed to intern string.\n");
        return 1;
    }

    char* another = new_string(&pool, "Another string.");
    if(!another){
        printf("Failed to intern another string.\n");
        return 1;
    }

    printf("Total strings interned: %zu\n", pool.count);
    printf("All interned strings:\n");
    for(size_t i = 0; i < pool.count; i++){
        printf(" - %s\n", pool.ptrs[i]);
    }

    free_string_pool(&pool);
    return 0;
}