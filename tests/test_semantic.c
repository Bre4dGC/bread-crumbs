#include <stdio.h>

#include "compiler/frontend/semantic.h"
#include "common/utils.h"

char* filepath;

int main(void)
{
    filepath = util_strdup("file_with_errors.brc");

    init_types();
    
    type_t* type_array = NULL;
    type_t* type_function = NULL;
    type_t* type_struct = NULL;
    
    type_array = new_type_array(type_int, 10);
    if(!type_array){
        fprintf(stderr, "Failed to create array type\n");
        goto failure;
        return EXIT_FAILURE;
    }
    printf("Array Type: kind=%d, size=%zu, align=%zu, elem_type_kind=%d, length=%zu\n",
           type_array->kind, type_array->size, type_array->align,
           type_array->array.elem_type->kind, type_array->array.length);

    type_function = new_type_function(type_void, NULL, 0);
    if(!type_function){
        fprintf(stderr, "Failed to create function type\n");
        goto failure;
        return EXIT_FAILURE;
    }
    printf("Function Type: kind=%d, size=%zu, align=%zu, return_type_kind=%d, param_count=%zu\n",
           type_function->kind, type_function->size, type_function->align,
           type_function->func.return_type->kind, type_function->func.param_count);

    type_struct = new_type_compound(NULL, 0);
    if(!type_struct){
        fprintf(stderr, "Failed to create struct type\n");
        goto failure;
        return EXIT_FAILURE;
    }
    printf("Struct Type: kind=%d, size=%zu, align=%zu, member_count=%zu\n",
           type_struct->kind, type_struct->size, type_struct->align,
           type_struct->compound.member_count);

    free_type(type_array);
    free_type(type_function);
    free_type(type_struct);
    
    free(type_array);
    free(type_function);
    free(type_struct);

    free_types();
    free(filepath);
    return EXIT_SUCCESS;

failure:
    if(type_array){
        free_type(type_array);
        free(type_array);
    }
    if(type_function){
        free_type(type_function);
        free(type_function);
    }
    if(type_struct){
        free_type(type_struct);
        free(type_struct);
    }
    free_types();
    free(filepath);
    return EXIT_FAILURE;
}
