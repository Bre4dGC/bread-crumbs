#pragma once

#include <wchar.h>
#include <stddef.h>

typedef enum {
    TYPE_WARNING,
    TYPE_ERROR,
    TYPE_FATAL,
} ErrorSeverityType;

typedef enum {
    LEXER_ERROR_UNCLOSED_STRING,
    LEXER_ERROR_UNMATCHED_PAREN,
    LEXER_ERROR_ILLEGAL_CHARACTER,
    LEXER_ERROR_UNEXPECTED_TOKEN,
    LEXER_ERROR_INVALID_NUMBER,
    LEXER_ERROR_INVALID_IDENTIFIER,
    // TODO: add more error types as needed
} LexerErrorType;


typedef enum {
    PARSER_ERROR_UNEXPECTED_TOKEN,
    PARSER_ERROR_INVALID_EXPRESSION,
    PARSER_ERROR_UNEXPECTED_END_OF_FILE,
    // TODO: add more error types as needed
} ParserErrorType;

typedef enum {
    ERROR_TYPE_LEXER,
    ERROR_TYPE_PARSER,
} ErrorTypeTag;

typedef struct {
    ErrorSeverityType severity_type;
    ErrorTypeTag type_tag;
    union {
        LexerErrorType lexer_error;
        ParserErrorType parser_error;
    };
    size_t line;
    size_t column;
    wchar_t *file_path;
    wchar_t *message;
} Error;

Error *error_new(ErrorSeverityType severity, ErrorTypeTag type, const int error_code, const size_t line, const size_t column, const wchar_t *file_path, const wchar_t *message);
void error_print(const Error *error);
void error_free(Error *error);