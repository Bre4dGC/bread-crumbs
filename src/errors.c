
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

Error *error_new(ErrorSeverityType severity, ErrorTypeTag type, const int error_code, const size_t line, const size_t column, const wchar_t *file_path, const wchar_t *message)
{
    Error *error = (Error *)malloc(sizeof(Error));
    if (!error) return NULL;

    error->severity_type = severity;
    error->type_tag = type;
    error->line = line;
    error->column = column;
    error->file_path = wcsdup(file_path);
    error->message = wcsdup(message);
    
    if (!error->file_path) {
        free(error);
        return NULL;
    }
    if (!error->message) {
        free(error);
        return NULL;
    }

    switch (type) {
        case ERROR_TYPE_LEXER:
            error->lexer_error = (LexerErrorType)error_code;
            break;
        case ERROR_TYPE_PARSER:
            error->parser_error = (ParserErrorType)error_code;
            break;
        default:
            error_free(error);
            return NULL;
    }

    return error;
}

void error_print(const Error *error)
{
    if (!error) return;

    const wchar_t *severity_str;
    switch (error->severity_type) {
        case TYPE_WARNING:
            severity_str = L"Warning";
            break;
        case TYPE_ERROR:
            severity_str = L"Error";
            break;
        case TYPE_FATAL:
            severity_str = L"Fatal";
            break;
        default:
            severity_str = L"Unknown";
    }

    const wchar_t *type_str;
    switch (error->type_tag) {
        case ERROR_TYPE_LEXER:
            type_str = L"Lexer Error";
            break;
        case ERROR_TYPE_PARSER:
            type_str = L"Parser Error";
            break;
        default:
            type_str = L"Unknown Error Type";
    }
    
    const wchar_t *file_path = error->file_path ? error->file_path : L"(unknown file)";
    const wchar_t *message = error->message ? error->message : L"(no message)";

    wprintf(L"path: %ls\n", file_path);
    wprintf(L"[%ls] %ls at %ls:%zu:%zu\n", severity_str, type_str, file_path, error->line, error->column);
    wprintf(L"%ls\n", message);
}

void error_free(Error *error)
{
    if (error) {
        free(error->message);
        free(error->file_path);
        free(error);
    }
}