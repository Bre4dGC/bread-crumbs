#include <stdio.h>
#include <sys/time.h>

#include "core/hashmap.h"
#include "core/common/source.h"
#include "core/common/benchmark.h"

int main(void)
{
    bench_start();

    hashmap_t* table = new_hashmap();

    int i_val = 42;
    const char* i_key = "int_key";
    hm_insert(table, i_key, &i_val);
    printf("Inserted key %s with value %d\n", i_key, i_val);

    char s_val[] = "str";
    const char* s_key = "str_key";
    hm_insert(table, s_key, s_val);
    printf("Inserted key %s with value %s\n", s_val, s_val);

    int* lookup_i = (int*)hm_lookup(table, i_key);
    if(lookup_i) printf("Lookup key %s: found value %d\n", i_key, *lookup_i);
    else printf("Lookup key %s: not found\n", i_key);

    char* lookup_s = (char*)hm_lookup(table, s_val);
    if(lookup_s) printf("Lookup key %s: found value %s\n", s_val, lookup_s);
    else printf("Lookup key %s: not found\n", s_val);

    hm_delete(table, i_key);
    printf("Deleted key %s\n", i_key);

    free_hashmap(table);

    bench_stop();
    bench_print("Test hash table");

    return 0;
}
