#include "core/arena.h"
#include "core/common/source.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    arena_t* arena = new_arena(ARENA_DEFAULT_SIZE);

    double* value = arena_alloc(arena, sizeof(double), alignof(double));
    if(!value){
        printf("Something wrong with value");
        return 1;
    }

    int* numbers = arena_alloc_array(arena, sizeof(int), 10, alignof(int));
    if(!numbers){
        printf("Something wrong with numbers");
        return 1;
    }

    char* string = arena_alloc_default(arena, 256);
    if(!string){
        printf("Something wrong with string");
        return 1;
    }

    *value = 3.1415;
    printf("Value: %f\n", *value);

    if(numbers && value && string){
        for(int i = 0; i < 10; i++){
            numbers[i] = i;
            printf("%d ", numbers[i]);
        }
    }

    strcpy(string, "hello world");
    printf("\nstring: %s\n", string);

    free_arena(arena);

    bm_stop();
    bm_print("Test arena");
    return 0;
}
