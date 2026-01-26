#include <stdio.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/core/string_pool.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();

    string_pool_t pool = new_string_pool(ARENA_DEFAULT_SIZE);

    string_t string = new_string(&pool, "Hello, World!");
    if(!string.data){
        printf("Failed to intern string.\n");
        return 1;
    }

    string_t another = new_string(&pool, "Another string.");
    if(!another.data){
        printf("Failed to intern another string.\n");
        return 1;
    }

    printf("Total strings interned: %zu\n", pool.count);
    printf("All interned strings:\n");

    for(size_t i = 0; i < pool.count; ++i){
        printf("%s\n", pool.elements[i].data);
    }

    free_string_pool(&pool);

    bench_stop();
    bench_print("Test string pool");

    return 0;
}
