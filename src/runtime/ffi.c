#include "runtime/ffi.h"

ffi_registry_t* new_ffi_registry(void)
{
    // TODO: Implement
    return NULL;
}

bool register_ffi_function(ffi_registry_t* registry, const char* name, void* function_ptr, size_t arg_count, const char** arg_types, const char* return_type)
{
    // TODO: Implement
    return false;
}

void* get_ffi_function(ffi_registry_t* registry, const char* name)
{
    // TODO: Implement
    return NULL;
}

void free_ffi_registry(ffi_registry_t* registry)
{
    // TODO: Implement
}
