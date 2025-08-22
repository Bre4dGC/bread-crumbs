#pragma once

#include <stddef.h>

enum error_severity{
    TYPE_WARNING, TYPE_ERROR, TYPE_FATAL
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
    PARSER_ERROR_EXPECTED_NAME,
    PARSER_ERROR_EXPECTED_TYPE,
    PARSER_ERROR_EXPECTED_PAREN,
    PARSER_ERROR_EXPECTED_OPERATOR,
    PARSER_ERROR_EXPECTED_KEYWORD,
    // TODO: add more error types as needed
};

enum error_type_tag{
    ERROR_TYPE_LEXER,
    ERROR_TYPE_PARSER,
};

struct error{
    enum error_severity severity_type;
    enum error_type_tag type_tag;
    union {
        enum lexer_error_type lexer_error;
        enum parser_error_type parser_error;
    };
    size_t line;
    size_t column;
    size_t length;
    char *input;
    char *message;
};

struct error* new_error(
    enum error_severity severity,
    enum error_type_tag type_tag,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char *input
);
void print_error(const struct error *error);
void free_error(struct error *error);