#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "core/ds/arena.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    arena_t* arena = new_arena(ARENA_DEF_SIZE);
    assert(arena != NULL);

    int* num1 = arena_alloc_default(arena, sizeof(int));
    assert(num1 != NULL);
    *num1 = 42;
    assert(*num1 == 42);

    char* str1 = arena_alloc_array(arena, sizeof(char), 6, alignof(char));
    assert(str1 != NULL);
    strcpy(str1, "Hello");
    assert(strcmp(str1, "Hello") == 0);

    printf("Used: %zu bytes\n", arena_used(arena));
    printf("Capacity: %zu bytes\n", arena_capacity(arena));
    printf("Usage: %.2f%%\n", (double)arena_used(arena) / arena_capacity(arena) * 100);

    printf("Number: %d\n", *num1);
    printf("String: %s\n", str1);

    free_arena(arena);

    bm_stop();
    bm_print("Arena tests");
    return 0;
}
