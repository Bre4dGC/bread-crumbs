#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/diagnostic.h"
#include "compiler/core/arena_alloc.h"
#include "compiler/core/string_pool.h"

const char* report_msg(const enum report_code code);

report_table_t* new_report_table(arena_t* arena)
{
    report_table_t* table = (report_table_t*)arena_alloc(arena, sizeof(report_table_t), alignof(report_table_t));
    if(!table) return NULL;

    table->arena = new_arena(DEFAULT_REPORT_POOL_SIZE * sizeof(report_t));
    if(!table->arena){
        return NULL;
    }

    table->string_pool = new_string_pool(DEFAULT_REPORT_POOL_SIZE);
    table->count = 0;

    return table;
}

void add_report(
    report_table_t* rt,
    const enum report_severity sev,
    const enum report_code code,
    const location_t loc,
    const size_t length,
    const char* input)
{
    if(!rt) return;
    const char* safe_input = input ? input : "";

    if(!arena_has_space(rt->arena, sizeof(report_t), alignof(report_t))){
        size_t cur = rt->arena->current ? rt->arena->current->capacity : 0;
        size_t need = sizeof(report_t) + (alignof(report_t) - 1);
        size_t new_cap = cur ? (cur * 2) : need;
        if(new_cap < need) new_cap = need;
        if(!arena_expand(rt->arena, new_cap)){
            return;
        }
    }

    report_t* store_report = arena_alloc(rt->arena, sizeof(report_t), alignof(report_t));
    if(!store_report) return;

    *store_report = (report_t){
        .severity = sev,
        .code = code,
        .loc = loc,
        .length = length,
        .input = new_string(&rt->string_pool, safe_input),
        .filepath = new_string(&rt->string_pool, "unknown")
    };
    rt->count += 1;
}

void print_report(const report_t* report)
{
    if(!report || !report->input.data) return;

    printf("\n %zu |\t%s\n", report->loc.line, report->input.data);
    printf(" %*s |\t%*s\033[31m",   (int)report->loc.line < 10   ? 1 :
                                    (int)report->loc.line < 100  ? 2 :
                                    (int)report->loc.line < 1000 ? 3 : 4,
                                    "", report->loc.column != 0 ? (int)report->loc.column - 1 : 0, "");

    if(report->length == 1){
        printf("^");
    }
    else for(size_t i = 0; i < report->length; ++i){
        printf("~");
    }

    printf(" \033[36m%s\033[0m", report_msg(report->code));

    printf("\n%s\033[0m %s at %zu:%zu\n",   report->severity == SEV_ERR  ? "\033[31m[ERROR]"   :
                                            report->severity == SEV_WARN ? "\033[33m[WARNING]" :
                                            report->severity == SEV_NOTE ? "\033[34m[NOTE]"    :
                                                                           "\033[31m[UNKNOWN]",
                                            report->filepath.data, report->loc.line, report->loc.column);
}

void print_report_table(const report_table_t* table)
{
    if(!table) return;

    size_t printed = 0;
    for(arena_block_t* b = table->arena->head; b && printed < table->count; b = b->next){
        const unsigned char* data = b->data;
        size_t offset = 0;
        while(offset + sizeof(report_t) <= b->offset && printed < table->count){
            report_t* report = (report_t*)(data + offset);
            print_report(report);
            offset += sizeof(report_t);
            printed++;
        }
    }
}

void free_report_table(report_table_t* table)
{
    if(!table) return;
    free_string_pool(&table->string_pool);
    free_arena(table->arena);
}

const char* report_msg(const enum report_code code)
{
    switch(code){
        case ERR_ILLEG_CHAR:    return "Illegal character";
        case ERR_INVAL_LIT:     return "Invalid literal";
        case ERR_INVAL_NUM:     return "Invalid number format";
        case ERR_INVAL_IDENT:   return "Invalid identifier";
        case ERR_INVAL_STR:     return "Invalid string";
        case ERR_UNCLO_STR:     return "Unclosed string literal";
        case ERR_UNMAT_PAREN:   return "Unmatched parenthesis";
        case ERR_UNEXP_EOF:     return "Unexpected end of file";
        case ERR_INVAL_ESCSEQ:  return "Invalid escape sequence";
        case ERR_UNEXP_TOKEN:   return "Unexpected token";
        case ERR_INVAL_EXPR:    return "Invalid expression";
        case ERR_EXPEC_IDENT:   return "Expected name";
        case ERR_EXPEC_TYPE:    return "Expected type";
        case ERR_EXPEC_PAREN:   return "Expected parenthesis";
        case ERR_EXPEC_OPER:    return "Expected operator";
        case ERR_EXPEC_KEYWORD: return "Expected keyword";
        case ERR_EXPEC_EXPR:    return "Expected expression";
        case ERR_EXPEC_PARAM:   return "Expected parameter";
        case ERR_INVAL_UNARYOP: return "Invalid unary operator";
        case ERR_TYPE_MISMATCH: return "Type mismatch";
        case ERR_UNDEC_VAR:     return "Undeclared variable";
        case ERR_UNDEC_FUNC:    return "Undeclared function";
        case ERR_VAR_ALREADY_DECL:  return "Variable already declared";
        case ERR_FUNC_ALREADY_DECL: return "Function already declared";
        case ERR_FAIL_TO_DECL_VAR:  return "Failed to declare variable";
        case ERR_FAIL_TO_DECL_FUNC: return "Failed to declare function";
        case ERR_INVAL_OPER:        return "Invalid operation";
        case ERR_INVAL_FUNC_CALL:   return "Invalid function call";
        case ERR_INVAL_ARG_COUNT:   return "Invalid argument count";
        case ERR_INVAL_ARG_TYPE:    return "Invalid argument type";
        case ERR_INVAL_RET_TYPE:    return "Invalid return type";
        case ERR_NOT_A_FUNC:        return "Not a function";
        case ERR_RET_OUTSIDE_FUNC:  return "Return outside function";
        case ERR_BREAK_OUTSIDE_LOOP:    return "Break outside loop";
        case ERR_UNIMPL_NODE:           return "Unimplemented node";
        case ERR_CONTINUE_OUTSIDE_LOOP: return "Continue outside loop";
        case ERR_VAR_NO_TYPE_OR_INITIALIZER: return "Variable has no type or initializer";
        default: return "Unknown report";
    }
}
