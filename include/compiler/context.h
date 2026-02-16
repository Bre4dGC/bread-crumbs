#include "core/arena.h"
#include "core/strings.h"
#include "core/hashmap.h"
#include "core/diagnostic.h"

#include "compiler/frontend/semantic.h"
#include "compiler/backend/codegen.h"

enum compile_phase {
    COMPILE_PHASE_LEXING,
    COMPILE_PHASE_PARSING,
    COMPILE_PHASE_ANALYSIS,
    COMPILE_PHASE_CODEGEN,
    COMPILE_PHASE_EXECUTION
};

typedef struct {
    string_t filename;
    string_t source;
} compiler_input_t;

typedef struct {
    bool debug;
    bool verbose;
    enum {NONE, SOFT, HARD} optimization;
} compiler_option_t;

typedef struct {
    arena_manager_t arena_manager;

    string_pool_t permanent_strings;
    string_pool_t temp_strings;

    hashmap_t global_idents;
    hashmap_t local_idents;
} compiler_memory_t;

typedef struct {
    enum compile_phase phase;

    compiler_input_t input;
    compiler_option_t options;
    compiler_memory_t memory;

    report_table_t reports;

    ast_t ast_root;
    symbol_table_t* symbols;
    ir_t ir;
    codegen_t codegen;
} compiler_context_t;

compiler_context_t* new_compiler_context(void);
void free_compiler_context(compiler_context_t* context);
