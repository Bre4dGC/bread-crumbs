#pragma once

#include <stddef.h>

enum report_severity {
    SEVERITY_NOTE, SEVERITY_WARNING, SEVERITY_ERROR
};

enum report_code {
    // LEXER
    ERROR_ILLEGAL_CHARACTER,
    ERROR_UNEXPECTED_END_OF_FILE,
    ERROR_INVALID_LITERAL,
    ERROR_INVALID_NUMBER,
    ERROR_INVALID_IDENTIFIER,
    ERROR_INVALID_STRING,
    ERROR_UNCLOSED_STRING,
    ERROR_UNMATCHED_PAREN,
    ERROR_INVALID_ESCAPE_SEQUENCE,
    
    // PARSER
    ERROR_UNEXPECTED_TOKEN,
    ERROR_INVALID_EXPRESSION,
    ERROR_INVALID_UNARY_OP,
    ERROR_EXPECTED_EXPRESSION,
    ERROR_EXPECTED_OPERATOR,
    ERROR_EXPECTED_IDENTIFIER,
    ERROR_EXPECTED_TYPE,
    ERROR_EXPECTED_PAREN,
    ERROR_EXPECTED_KEYWORD,
    ERROR_EXPECTED_DELIMITER,
    ERROR_EXPECTED_PARAM,
    
    // SEMANTIC
    ERROR_TYPE_MISMATCH,
    ERROR_UNDECLARED_VARIABLE,
    ERROR_UNDECLARED_FUNCTION,
    ERROR_VARIABLE_ALREADY_DECLARED,
    ERROR_FUNCTION_ALREADY_DECLARED,
    ERROR_FAILED_TO_DECLARE_VARIABLE,
    ERROR_FAILED_TO_DECLARE_FUNCTION,
    ERROR_INVALID_OPERATION,
    ERROR_INVALID_FUNCTION_CALL,
    ERROR_INVALID_ARGUMENT_COUNT,
    ERROR_INVALID_ARGUMENT_TYPE,
    ERROR_INVALID_RETURN_TYPE,
    ERROR_NOT_A_FUNCTION,
    ERROR_BREAK_OUTSIDE_LOOP,
    ERROR_UNIMPLEMENTED_NODE,
    ERROR_CONTINUE_OUTSIDE_LOOP,
    ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER,
};

typedef struct {
    enum report_severity severity;
    enum report_code code;
    
    size_t line;
    size_t column;
    size_t length;

    char* input;
    char* message;
    char* filepath;
} report_t;

typedef struct {
    report_t* elements;
    size_t count;
} report_table_t;

report_t* new_report(
    const enum report_severity severity,
    const enum report_code code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char* input);
void print_report(const report_t* report);
void free_report(report_t* report);
