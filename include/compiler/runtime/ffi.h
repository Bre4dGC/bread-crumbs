#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* name;
    void* function_ptr;
    size_t arg_count;
    const char** arg_types;
    const char* return_type;
} ffi_function_t;

typedef struct {
    ffi_function_t* functions;
    size_t function_count;
    size_t function_capacity;
} ffi_registry_t;

ffi_registry_t* new_ffi_registry(void);
bool register_ffi_function(ffi_registry_t* registry, const char* name, void* function_ptr, size_t arg_count, const char** arg_types, const char* return_type);
void* get_ffi_function(ffi_registry_t* registry, const char* name);
void free_ffi_registry(ffi_registry_t* registry);
