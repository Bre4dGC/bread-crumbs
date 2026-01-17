#include <stdbool.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/core/string_pool.h"
#include "compiler/core/hash_table.h"
#include "compiler/core/diagnostic.h"

#include "compiler/frontend/semantic.h"
#include "compiler/middle/ir.h"

enum compile_phase {
    COMPILE_PHASE_LEXING,
    COMPILE_PHASE_PARSING,
    COMPILE_PHASE_ANALYSIS,
    COMPILE_PHASE_CODEGEN,
    COMPILE_PHASE_EXECUTION
};

typedef struct {
    const char* filename;
    char* source;
    size_t source_len;
} compiler_input_t;

typedef struct {
    arena_manager_t arena_manager;
    
    string_pool_t permanent_strings;
    string_pool_t temp_strings;

    hash_table_t global_idents;
    hash_table_t local_idents;
} compiler_memory_t;

typedef struct {
    bool debug;
    bool verbose;
    enum {NONE, SOFT, HARD} optimization;
} compiler_options_t;

// typedef struct {

// } compiler_toolchain_t;

typedef struct {
    enum compile_phase phase;

    compiler_input_t input;
    compiler_memory_t memory;
    compiler_options_t options;

    report_table_t reports;

    arena_t ast_root;
    semantic_t semantic;
    virtual_machine_t vm;
} compiler_context_t;

compiler_memory_t* new_compiler_memory(void);
void free_compiler_memory(compiler_memory_t* mem);

compiler_context_t* new_compiler_context(void);
void free_compiler_context(compiler_context_t* ctx);
