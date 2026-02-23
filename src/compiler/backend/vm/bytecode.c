#include "compiler/backend/vm/bytecode.h"

bytecode_t* new_bytecode(size_t length);
void free_bytecode(bytecode_t* bytecode);
