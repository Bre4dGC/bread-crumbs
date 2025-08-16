#pragma once

#include <wchar.h>
#include <stddef.h>

enum err_severity_type{
    TYPE_WARNING, ERROR_SEVERITY_TYPE
};

enum lexer_error_type{
    LEXER_ERROR_UNCLOSED_STRING,
    LEXER_ERROR_UNMATCHED_PAREN,
    LEXER_ERROR_ILLEGAL_CHARACTER,
    LEXER_ERROR_INVALID_NUMBER,
    LEXER_ERROR_INVALID_IDENTIFIER,
    LEXER_ERROR_INVALID_STRING,
    // TODO: add more error types as needed
};


enum parser_error_type{
    PARSER_ERROR_UNEXPECTED_TOKEN,
    PARSER_ERROR_INVALID_EXPRESSION,
    PARSER_ERROR_UNEXPECTED_END_OF_FILE,
    // TODO: add more error types as needed
};

enum err_type_tag{
    ERROR_TYPE_LEXER,
    ERROR_TYPE_PARSER,
};

struct error{
    enum err_severity_type severity_type;
    enum err_type_tag type_tag;
    union {
        enum lexer_error_type lexer_error;
        enum parser_error_type parser_error;
    };
    size_t line;
    size_t column;
    size_t length;
    wchar_t *input;
    wchar_t *message;
};

struct error* new_error(
    enum err_severity_type severity,
    enum err_type_tag type_tag,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const wchar_t *input
);
void print_error(const struct error *error);
void free_error(struct error *error);