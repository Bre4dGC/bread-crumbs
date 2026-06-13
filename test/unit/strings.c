#include "core/ds/arena.h"
#include "core/ds/strings.h"

#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    string_pool_t pool = new_string_pool(ARENA_DEF_SIZE);

    string_t str1 = new_string(&pool, "Hello, World!");
    string_t str2 = new_string(&pool, "Hello, World!"); // Should reuse the same string
    string_t str3 = new_string(&pool, "Goodbye!");

    printf("String 1: %s (hash: %u)\n", str1.data, str1.hash);
    printf("String 2: %s (hash: %u)\n", str2.data, str2.hash);
    printf("String 3: %s (hash: %u)\n", str3.data, str3.hash);

    free_string_pool(&pool);

    bm_stop();
    bm_print("Test string pool");

    return 0;
}
