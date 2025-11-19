#include "compiler/core/arena_alloc.h"
#include "compiler/core/string_pool.h"
#include "compiler/core/diagnostic.h"

#include "compiler/frontend/lexer.h"
#include "compiler/frontend/parser.h"
#include "compiler/frontend/semantic.h"

#include "compiler/backend/vm.h"

enum compile_phase {
    COMPILE_PHASE_LEXING,
    COMPILE_PHASE_PARSING,
    COMPILE_PHASE_SEMANTIC_ANALYSIS,
    COMPILE_PHASE_CODE_GENERATION,
    COMPILE_PHASE_EXECUTION 
};

typedef struct {
    enum compile_phase phase;

    lexer_t* lexer;
    parser_t* parser;
    semantic_context_t* semantic;
    virtual_machine_t* vm;

    report_table_t diagnostics;
} compiler_context_t;
