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

