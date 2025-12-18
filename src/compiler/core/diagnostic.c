
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/core/diagnostic.h"
#include "common/utils.h"

const char* report_msg(const enum report_code code);

report_t* new_report(
    const enum report_severity severity,
    const enum report_code code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char* input)
{

    report_t* report = (report_t*)malloc(sizeof(report_t));
    if (!report) return NULL;

    report->severity = severity;
    report->code = code;
    report->line = line;
    report->column = column;
    report->length = length;
    report->input = input ? util_strdup(input) : "NO INPUT";
    report->message = util_strdup(report_msg(code));
    report->filepath = util_strdup("unknown");

    if((input && !report->input) || !report->message){
        free_report(report);
        return NULL;
    }

    return report;
}

void print_report(const report_t* report)
{
    if (!report || !report->input) return;

    printf("\n %zu |\t%s\n", report->line, report->input);
    printf(" %*s |\t%*s\033[31m",   (int)report->line < 10   ? 1 :
                                    (int)report->line < 100  ? 2 :
                                    (int)report->line < 1000 ? 3 : 4,
                                    "", report->column != 0 ? (int)report->column - 1 : 0, "");

    if(report->length == 1){
        printf("^");
    }
    else for(size_t i = 0; i < report->length; ++i){
        printf("~");
    }

    printf(" \033[36m%s\033[0m", report->message);

    printf("\n%s\033[0m %s at %zu:%zu\n",   report->severity == SEVERITY_ERROR   ? "\033[31m[ERROR]"   :
                                            report->severity == SEVERITY_WARNING ? "\033[33m[WARNING]" :
                                            report->severity == SEVERITY_NOTE    ? "\033[34m[NOTE]"    :
                                                                                   "\033[31m[UNKNOWN]",
                                            report->filepath, report->line, report->column);
}

void free_report(report_t *report)
{
    if (!report) return;
    free(report->input);
    free(report->message);
    free(report->filepath);

    report->input = NULL;
    report->message = NULL;
    report->filepath = NULL;

    free(report);
    report = NULL;
}

const char* report_msg(const enum report_code code)
{
    switch(code){
        case ERROR_ILLEGAL_CHARACTER:  return "Illegal character";
        case ERROR_INVALID_LITERAL:    return "Invalid literal";
        case ERROR_INVALID_NUMBER:     return "Invalid number format";
        case ERROR_INVALID_IDENTIFIER: return "Invalid identifier";
        case ERROR_INVALID_STRING:     return "Invalid string";
        case ERROR_UNCLOSED_STRING:    return "Unclosed string literal";
        case ERROR_UNMATCHED_PAREN:    return "Unmatched parenthesis";
        case ERROR_UNEXPECTED_END_OF_FILE:  return "Unexpected end of file";
        case ERROR_INVALID_ESCAPE_SEQUENCE: return "Invalid escape sequence";
        case ERROR_UNEXPECTED_TOKEN:    return "Unexpected token";
        case ERROR_INVALID_EXPRESSION:  return "Invalid expression";
        case ERROR_EXPECTED_IDENTIFIER: return "Expected name";
        case ERROR_EXPECTED_TYPE:       return "Expected type";
        case ERROR_EXPECTED_PAREN:      return "Expected parenthesis";
        case ERROR_EXPECTED_OPERATOR:   return "Expected operator";
        case ERROR_EXPECTED_KEYWORD:    return "Expected keyword";
        case ERROR_EXPECTED_EXPRESSION: return "Expected expression";
        case ERROR_EXPECTED_PARAM:      return "Expected parameter";
        case ERROR_INVALID_UNARY_OP:    return "Invalid unary operator";
        case ERROR_TYPE_MISMATCH:       return "Type mismatch";
        case ERROR_UNDECLARED_VARIABLE: return "Undeclared variable";
        case ERROR_UNDECLARED_FUNCTION: return "Undeclared function";
        case ERROR_VARIABLE_ALREADY_DECLARED: return "Variable already declared";
        case ERROR_INVALID_OPERATION:      return "Invalid operation";
        case ERROR_INVALID_FUNCTION_CALL:  return "Invalid function call";
        case ERROR_INVALID_ARGUMENT_COUNT: return "Invalid argument count";
        case ERROR_INVALID_ARGUMENT_TYPE:  return "Invalid argument type";
        case ERROR_INVALID_RETURN_TYPE:    return "Invalid return type";
        case ERROR_NOT_A_FUNCTION:         return "Not a function";
        case ERROR_BREAK_OUTSIDE_LOOP:     return "Break outside loop";
        case ERROR_UNIMPLEMENTED_NODE:     return "Unimplemented node";
        case ERROR_CONTINUE_OUTSIDE_LOOP:  return "Continue outside loop";
        case ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER: return "Variable has no type or initializer";
        default: return "Unknown report";
    }
}
