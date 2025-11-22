#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler/core/hash_table.h"

int main(void)
{
    srand(time(NULL));

    hash_table_t* table = new_hashtable();
    printf("Created hashtable at %p\n", table);
    
    int i_val = 42;
    int i_key = rand();
    ht_insert(table, i_key, &i_val);
    printf("Inserted key %d with value %d\n", i_key, i_val);
    
    char* s_val = "str";
    int s_key = rand();
    ht_insert(table, s_key, s_val);
    printf("Inserted key %d with value %s\n", s_key, s_val);

    int* lookup_i = (int*)ht_lookup(table, i_key);
    if(lookup_i) printf("Lookup key %d: found value %d\n", i_key, *lookup_i);
    else printf("Lookup key %d: not found\n", i_key);

    char* lookup_s = (char*)ht_lookup(table, s_key);
    if(lookup_s) printf("Lookup key %d: found value %s\n", s_key, lookup_s);
    else printf("Lookup key %d: not found\n", s_key);

    ht_delete(table, i_key);
    printf("Deleted key %d\n", i_key);

    free_hashtable(table);
    return 0;
}