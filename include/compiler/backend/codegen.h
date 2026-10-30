#include <stdio.h>
#pragma once

#include <llvm-c/Core.h> // LLVMContextRef, LLVMModuleRef, LLVMBuilderRef

#include "core/ds/hashmap.h"    // hashmap_t
#include "compiler/middle/ir.h" // ir_t

typedef struct {
    LLVMContextRef ctx;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    hashmap_t* named_values;
} codegen_t;

codegen_t* codegen_new(void);
void codegen_free(codegen_t* cg);
bool codegen_emit(codegen_t* cg, ir_t* ir);
void codegen_dump(codegen_t* cg, FILE* out);
bool codegen_write_ll(codegen_t* cg, const char* path);
bool codegen_write_bitcode(codegen_t* cg, const char* path);
