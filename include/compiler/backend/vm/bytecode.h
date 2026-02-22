#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    // TODO: define
} bytecode_t;

bytecode_t* new_bytecode(size_t length);
void free_bytecode(bytecode_t* bytecode);