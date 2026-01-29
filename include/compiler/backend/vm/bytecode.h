#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* code;
    size_t length;
    size_t capacity;
} bytecode_t;

bytecode_t* new_bytecode(size_t length);
void free_bytecode(bytecode_t* bytecode);