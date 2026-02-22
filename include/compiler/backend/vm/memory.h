#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    // TODO: define
} vm_memory_t;

vm_memory_t* new_vm_memory(size_t capacity);
void free_vm_memory(vm_memory_t* memory);
void vm_memory_set(vm_memory_t* memory, size_t index, uint8_t value);
uint8_t vm_memory_get(vm_memory_t* memory, size_t index);