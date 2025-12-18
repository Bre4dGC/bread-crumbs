#include <stdio.h>
#include <sys/time.h>

#include "compiler/core/hash_table.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();

    hash_table_t* table = new_hashtable();
    
    int i_val = 42;
    char* i_key = "int_key";
    ht_insert(table, i_key, &i_val);
    printf("Inserted key %s with value %d\n", i_key, i_val);
    
    char* s_val = "str";
    ht_insert(table, s_val, s_val);
    printf("Inserted key %s with value %s\n", s_val, s_val);

    int* lookup_i = (int*)ht_lookup(table, i_key);
    if(lookup_i) printf("Lookup key %s: found value %d\n", i_key, *lookup_i);
    else printf("Lookup key %s: not found\n", i_key);

    char* lookup_s = (char*)ht_lookup(table, s_val);
    if(lookup_s) printf("Lookup key %s: found value %s\n", s_val, lookup_s);
    else printf("Lookup key %s: not found\n", s_val);

    ht_delete(table, i_key);
    printf("Deleted key %s\n", i_key);

    free_hashtable(table);

    bench_stop();
    bench_print("Test hash table");

    return 0;
}
