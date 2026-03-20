#pragma once

#include <stddef.h>     // size_t

#include "core/ds/strings.h"    // string_pool_t
#include "core/ds/arena.h"      // arena_t
#include "core/lang/source.h"   // location_t

#define MAX_REPORTS_COUNT        512
#define DEFAULT_REPORT_POOL_SIZE 32
#define DEF_ERR_LEN              1
#define MAX_ERR_LEN              64

enum report_severity { SEV_NOTE, SEV_WARN, SEV_ERR };

enum report_code {
    // LEXER
    ERR_ILLEG_CHAR,
    ERR_UNEXP_EOF,
    ERR_INVAL_LIT,
    ERR_INVAL_NUM,
    ERR_INVAL_IDENT,
    ERR_INVAL_STR,
    ERR_UNCLO_STR,
    ERR_UNMAT_PAREN,
    ERR_INVAL_ESCSEQ,

    // PARSER
    ERR_UNEXP_TOKEN,
    ERR_INVAL_EXPR,
    ERR_INVAL_UNARYOP,
    ERR_EXPEC_EXPR,
    ERR_EXPEC_OPER,
    ERR_EXPEC_IDENT,
    ERR_EXPEC_TYPE,
    ERR_EXPEC_PAREN,
    ERR_EXPEC_KEYWORD,
    ERR_EXPEC_DELIM,
    ERR_EXPEC_PARAM,

    // SEMANTIC
    ERR_TYPE_MISMATCH,
    ERR_UNDEC_VAR,
    ERR_UNDEC_FUNC,
    ERR_VAR_ALREADY_DECL,
    ERR_FUNC_ALREADY_DECL,
    ERR_FAIL_TO_DECL_VAR,
    ERR_FAIL_TO_DECL_FUNC,
    ERR_INVAL_OPER,
    ERR_INVAL_FUNC_CALL,
    ERR_INVAL_ARG_COUNT,
    ERR_INVAL_ARG_TYPE,
    ERR_INVAL_RET_TYPE,
    ERR_NOT_A_FUNC,
    ERR_RET_OUTSIDE_FUNC,
    ERR_BREAK_OUTSIDE_LOOP,
    ERR_UNIMPL_NODE,
    ERR_CONTINUE_OUTSIDE_LOOP,
    ERR_VAR_NO_TYPE_OR_INITIALIZER,
};

typedef struct {
    string_t* filename;
    string_t* line;

    enum report_severity severity;
    enum report_code code;

    location_t loc;
} report_t;

typedef struct {
    arena_t* arena;
    string_pool_t string_pool;
    size_t count;
} report_table_t;

void add_report(
    report_table_t* table,
    const source_t* src,
    const enum report_severity sev,
    const enum report_code code,
    const location_t loc
);
report_table_t* new_report_table(arena_t* arena);
void print_report_table(const report_table_t* table);
void free_report_table(report_table_t* table);
