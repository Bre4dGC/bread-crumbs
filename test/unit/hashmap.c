#include "core/ds/hashmap.h"

#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    hashmap_t* table = new_hashmap();
    
    char* keys[] = {"apple", "banana", "orange", "grape", "melon"};
    char* values[] = {"red", "yellow", "orange", "purple", "green"};

    for(size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++){
        hm_insert(table, keys[i], values[i]);
    }

    for(size_t i = 0; i < sizeof(keys)/sizeof(keys[0]); i++){
        char* value = hm_lookup(table, keys[i]);
        if(value){
            printf("%s: %s\n", keys[i], value);
        }
        else {
            printf("%s: not found\n", keys[i]);
        }
    }

    free_hashmap(table);

    bm_stop();
    bm_print("Test hash table");

    return 0;
}
