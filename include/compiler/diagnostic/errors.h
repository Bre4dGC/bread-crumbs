#pragma once

#include <stddef.h>

enum error_severity {
    SEVERITY_NOTE, SEVERITY_WARNING, SEVERITY_ERROR
};

enum lexer_error_type {
    LEXER_ERROR_ILLEGAL_CHARACTER,
    LEXER_ERROR_UNEXPECTED_END_OF_FILE,
    LEXER_ERROR_INVALID_LITERAL,
    LEXER_ERROR_INVALID_NUMBER,
    LEXER_ERROR_INVALID_IDENTIFIER,
    LEXER_ERROR_INVALID_STRING,
    LEXER_ERROR_UNCLOSED_STRING,
    LEXER_ERROR_UNMATCHED_PAREN,
    LEXER_ERROR_INVALID_ESCAPE_SEQUENCE,
};

enum parser_error_type {
    PARSER_ERROR_UNEXPECTED_TOKEN,
    PARSER_ERROR_INVALID_EXPRESSION,
    PARSER_ERROR_INVALID_UNARY_OP,
    PARSER_ERROR_EXPECTED_EXPRESSION,
    PARSER_ERROR_EXPECTED_OPERATOR,
    PARSER_ERROR_EXPECTED_IDENTIFIER,
    PARSER_ERROR_EXPECTED_TYPE,
    PARSER_ERROR_EXPECTED_PAREN,
    PARSER_ERROR_EXPECTED_KEYWORD,
    PARSER_ERROR_EXPECTED_DELIMITER,
    PARSER_ERROR_EXPECTED_PARAM,
};

enum semantic_error_type {
    SEMANTIC_ERROR_TYPE_MISMATCH,
    SEMANTIC_ERROR_UNDECLARED_VARIABLE,
    SEMANTIC_ERROR_UNDECLARED_FUNCTION,
    SEMANTIC_ERROR_VARIABLE_ALREADY_DECLARED,
    SEMANTIC_ERROR_FUNCTION_ALREADY_DECLARED,
    SEMANTIC_ERROR_FAILED_TO_DECLARE_VARIABLE,
    SEMANTIC_ERROR_FAILED_TO_DECLARE_FUNCTION,
    SEMANTIC_ERROR_INVALID_OPERATION,
    SEMANTIC_ERROR_INVALID_FUNCTION_CALL,
    SEMANTIC_ERROR_INVALID_ARGUMENT_COUNT,
    SEMANTIC_ERROR_INVALID_ARGUMENT_TYPE,
    SEMANTIC_ERROR_INVALID_RETURN_TYPE,
    SEMANTIC_ERROR_NOT_A_FUNCTION,
    SEMANTIC_ERROR_BREAK_OUTSIDE_LOOP,
    SEMANTIC_ERROR_UNIMPLEMENTED_NODE,
    SEMANTIC_ERROR_CONTINUE_OUTSIDE_LOOP,
    SEMANTIC_ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER,
};

enum error_phase {
    ERROR_TYPE_LEXER,
    ERROR_TYPE_PARSER,
    ERROR_TYPE_SEMANTIC
};

struct error {
    enum error_severity severity;
    enum error_phase phase;
    union {
        enum lexer_error_type lexer_error;
        enum parser_error_type parser_error;
        enum semantic_error_type semantic_error;
    };
    size_t line;
    size_t column;
    size_t length;
    char* input;
    char* message;
    char* filepath;
};

struct error_table {
    struct error* elements;
    size_t count;
    size_t capacity;
};

struct error* new_error(
    const enum error_severity severity,
    const enum error_phase phase,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char* input);
void print_error(const struct error *error);
void free_error(struct error *error);
