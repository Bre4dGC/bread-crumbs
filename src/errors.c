
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "errors.h"

static const wchar_t* error_msg(const enum err_type_tag type_tag, const int error_code) {
    switch (type_tag) {
        case ERROR_TYPE_LEXER:
            switch ((enum lexer_error_type)error_code) {
                case LEXER_ERROR_UNCLOSED_STRING: return L"Unclosed string literal";
                case LEXER_ERROR_UNMATCHED_PAREN: return L"Unmatched parenthesis";
                case LEXER_ERROR_ILLEGAL_CHARACTER: return L"Illegal character";
                case LEXER_ERROR_INVALID_NUMBER: return L"Invalid number format";
                case LEXER_ERROR_INVALID_IDENTIFIER: return L"Invalid identifier";
                default: return L"Unknown lexer error";
            }
        case ERROR_TYPE_PARSER:
            switch ((enum parser_error_type)error_code) {
                case PARSER_ERROR_UNEXPECTED_TOKEN: return L"Unexpected token";
                case PARSER_ERROR_INVALID_EXPRESSION: return L"Invalid expression";
                case PARSER_ERROR_UNEXPECTED_END_OF_FILE: return L"Unexpected end of file";
                default: return L"Unknown parser error";
            }
        default: return L"Unknown error type";
    }
}

struct error* new_error(
    enum err_severity_type severity,
    enum err_type_tag type_tag,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const wchar_t *input){

    struct error* err = (struct error*)malloc(sizeof(struct error));
    if (!err) return NULL;

    err->severity_type = severity;
    err->type_tag = type_tag;
    err->line = line;
    err->column = column;
    err->length = length;
    err->input = input ? wcsdup(input) : NULL;
    err->message = wcsdup(error_msg(type_tag, error_code));
    
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

    const wchar_t* severity_str = L"Unknown";
    switch (err->severity_type) {
        case TYPE_WARNING: severity_str = L"Warning"; break;
        case ERROR_SEVERITY_TYPE:   severity_str = L"Error";   break;
        default:           severity_str = L"Unknown"; break;
    }

    const wchar_t* message = err->message ? err->message : L"(no message)";
    const wchar_t* input = err->input ? err->input : L"";

    wprintf(L"\n%ls\n", input);

    if (err->column > 0) {
        for (size_t i = 1; i < err->column; ++i) putwchar(L' ');
    }

    if (err->length == 1) wprintf(L"^");
    else for (size_t i = 0; i < err->length; ++i) wprintf(L"~");

    wprintf(L"\n[%ls] %ls at %zu:%zu\n", severity_str, message, err->line, err->column);
}

void free_error(struct error *err)
{
    if (err) {
        free(err->input);
        free(err->message);
        free(err);
    }
}