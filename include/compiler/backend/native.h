#pragma once

#include "compiler/backend/codegen.h"

typedef struct native native_t;

native_t* native_create(void);
void native_free(native_t* nat);
bool native_emit_object(native_t* nat, codegen_t* cg, const char* output_path);
