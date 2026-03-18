#pragma once

#include <stddef.h>     // size_t
#include <stdint.h>     // uint8_t

void optimize_bytecode(uint8_t* bytecode, size_t length);
void eliminate_dead_code(uint8_t* bytecode, size_t length);
void inline_functions(uint8_t* bytecode, size_t length);
void constant_folding(uint8_t* bytecode, size_t length);
void dead_code_elimination(uint8_t* bytecode, size_t length);
void constant_propagation(uint8_t* bytecode, size_t length);
