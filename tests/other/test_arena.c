#include <stdio.h>
#include <string.h>
#include "compiler/core/arena_alloc.h"

int main(void)
{
    arena_t arena = new_arena(ARENA_DEFAULT_SIZE);
    if(!arena.data) fprintf(stderr, "Error: Could not allocate arena\n");

    printf("size: %lu\n", arena.size);
    printf("address: %p\n", arena.data);

    int i = 42;
    int* p = (int*)arena_alloc(&arena, sizeof(int), alignof(int));

    memcpy(p, &i, sizeof(int));

    printf("data: %d at %p\n", *p, p);

    free_arena(&arena);
    return 0;
}
