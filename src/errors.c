
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

static const char* error_msg(const enum error_type_tag type_tag, const int error_code) {
    switch (type_tag) {
        case ERROR_TYPE_LEXER:
            switch ((enum lexer_error_type)error_code) {
                case LEXER_ERROR_UNCLOSED_STRING: return "Unclosed string literal";
                case LEXER_ERROR_UNMATCHED_PAREN: return "Unmatched parenthesis";
                case LEXER_ERROR_ILLEGAL_CHARACTER: return "Illegal character";
                case LEXER_ERROR_INVALID_NUMBER: return "Invalid number format";
                case LEXER_ERROR_INVALID_IDENTIFIER: return "Invalid identifier";
                default: return "Unknown lexer error";
            }
        case ERROR_TYPE_PARSER:
            switch ((enum parser_error_type)error_code) {
                case PARSER_ERROR_UNEXPECTED_TOKEN: return "Unexpected token";
                case PARSER_ERROR_INVALID_EXPRESSION: return "Invalid expression";
                case PARSER_ERROR_UNEXPECTED_END_OF_FILE: return "Unexpected end of file";
                default: return "Unknown parser error";
            }
        default: return "Unknown error type";
    }
}

struct error* new_error(
    enum error_severity severity,
    enum error_type_tag type_tag,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char *input){

    struct error* err = (struct error*)malloc(sizeof(struct error));
    if (!err) return NULL;

    err->severity_type = severity;
    err->type_tag = type_tag;
    err->line = line;
    err->column = column;
    err->length = length;
    err->input = input ? strdup(input) : NULL;
    err->message = strdup(error_msg(type_tag, error_code));
    
    if ((input && !err->input) || !err->message) {
        free_error(err);
        return NULL;
    }

    switch (type_tag) {
        case ERROR_TYPE_LEXER:
            err->lexer_error = (enum lexer_error_type)error_code;
            break;
        case ERROR_TYPE_PARSER:
            err->parser_error = (enum parser_error_type)error_code;
            break;
        default:
            free_error(err);
            return NULL;
    }

    return err;
}

void print_error(const struct error* err) {
    if (!err) return;

    printf("\n%s\n", err->input ? err->input : "");

    printf("%*s", err->column != 0 ? (int)err->column - 1 : 0, "");

    if (err->length == 1) putchar('^');
    else for (size_t i = 0; i < err->length; ++i) putchar('~');

    switch (err->severity_type) {
        case TYPE_WARNING:
            printf("\n\033[35m[WARNING]\033[0m %s at %zu:%zu\n", err->message ? err->message : "", err->line, err->column);
            break;
        case TYPE_ERROR:
            printf("\n\033[33m[ERROR]\033[0m %s at %zu:%zu\n", err->message ? err->message : "", err->line, err->column);
            break;
        case TYPE_FATAL:
            printf("\n\033[31m[FATAL]\033[0m %s at %zu:%zu\n", err->message ? err->message : "", err->line, err->column);
            break;
        default:
            printf("\n[UNKNOW] %s at %zu:%zu\n", err->message ? err->message : "", err->line, err->column);
            break;
    }
}

void free_error(struct error *err)
{
    if (err) {
        free(err->input);
        free(err->message);
        free(err);
    }
}