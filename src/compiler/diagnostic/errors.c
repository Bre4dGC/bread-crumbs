
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/diagnostic/errors.h"
#include "common/utils.h"

extern char* filepath;

const char* error_msg(const enum error_phase phase, const int error_code);

struct error* new_error(
    const enum error_severity severity,
    const enum error_phase phase,
    const int error_code,
    const size_t line,
    const size_t column,
    const size_t length,
    const char *input)
{

    struct error* err = (struct error*)malloc(sizeof(struct error));
    if (!err) return NULL;

    err->severity = severity;
    err->phase = phase;
    err->line = line;
    err->column = column;
    err->length = length;
    err->input = input ? util_strdup(input) : NULL;
    err->message = util_strdup(error_msg(phase, error_code));
    err->filepath = util_strdup(filepath);

    if((input && !err->input) || !err->message){
        free_error(err);
        return NULL;
    }

    switch(phase){
        case ERROR_TYPE_LEXER: err->lexer_error = (enum lexer_error_type)error_code; break;
        case ERROR_TYPE_PARSER: err->parser_error = (enum parser_error_type)error_code; break;
        case ERROR_TYPE_SEMANTIC: err->semantic_error = (enum semantic_error_type)error_code; break;
        default: free_error(err); return NULL;
    }

    return err;
}

void print_error(const struct error* err)
{
    if (!err || !err->input) return;

    printf("\n %zu |\t%s\n", err->line, err->input);
    printf(" %*s |\t%*s\033[31m",   (int)err->line < 10   ? 1 :
                                    (int)err->line < 100  ? 2 :
                                    (int)err->line < 1000 ? 3 : 4,
                                    "", err->column != 0 ? (int)err->column - 1 : 0, "");

    if(err->length == 1){
        printf("^");
    }
    else for(size_t i = 0; i < err->length; ++i){
        printf("~");
    }

    printf(" \033[36m%s\033[0m", err->message);

    printf("\n%s\033[0m %s at %zu:%zu\n",   err->severity == SEVERITY_ERROR   ? "\033[31m[ERROR]"   :
                                            err->severity == SEVERITY_WARNING ? "\033[33m[WARNING]" :
                                            err->severity == SEVERITY_NOTE    ? "\033[34m[NOTE]"    :
                                                                                "\033[31m[UNKNOWN]",
                                            err->filepath, err->line, err->column);
}

void free_error(struct error *err)
{
    if (!err) return;
    free(err->input);
    free(err->message);
    free(err->filepath);

    err->input = NULL;
    err->message = NULL;
    err->filepath = NULL;

    free(err);
    err = NULL;
}

const char* error_msg(const enum error_phase phase, const int error_code)
{
    switch (phase) {
        case ERROR_TYPE_LEXER:
            switch((enum lexer_error_type)error_code){
                case LEXER_ERROR_ILLEGAL_CHARACTER:  return "Illegal character";
                case LEXER_ERROR_INVALID_LITERAL:    return "Invalid literal";
                case LEXER_ERROR_INVALID_NUMBER:     return "Invalid number format";
                case LEXER_ERROR_INVALID_IDENTIFIER: return "Invalid identifier";
                case LEXER_ERROR_INVALID_STRING:     return "Invalid string";
                case LEXER_ERROR_UNCLOSED_STRING:    return "Unclosed string literal";
                case LEXER_ERROR_UNMATCHED_PAREN:    return "Unmatched parenthesis";
                case LEXER_ERROR_UNEXPECTED_END_OF_FILE:  return "Unexpected end of file";
                case LEXER_ERROR_INVALID_ESCAPE_SEQUENCE: return "Invalid escape sequence";
                default: return "Unknown lexer error";
            }
        case ERROR_TYPE_PARSER:
            switch((enum parser_error_type)error_code){
                case PARSER_ERROR_UNEXPECTED_TOKEN:    return "Unexpected token";
                case PARSER_ERROR_INVALID_EXPRESSION:  return "Invalid expression";
                case PARSER_ERROR_EXPECTED_IDENTIFIER: return "Expected name";
                case PARSER_ERROR_EXPECTED_TYPE:       return "Expected type";
                case PARSER_ERROR_EXPECTED_PAREN:      return "Expected parenthesis";
                case PARSER_ERROR_EXPECTED_OPERATOR:   return "Expected operator";
                case PARSER_ERROR_EXPECTED_KEYWORD:    return "Expected keyword";
                case PARSER_ERROR_EXPECTED_EXPRESSION: return "Expected expression";
                case PARSER_ERROR_EXPECTED_PARAM:      return "Expected parameter";
                case PARSER_ERROR_INVALID_UNARY_OP:    return "Invalid unary operator";
                default: return "Unknown parser error";
            }
        case ERROR_TYPE_SEMANTIC:
            switch ((enum semantic_error_type)error_code) {
                case SEMANTIC_ERROR_TYPE_MISMATCH:       return "Type mismatch";
                case SEMANTIC_ERROR_UNDECLARED_VARIABLE: return "Undeclared variable";
                case SEMANTIC_ERROR_UNDECLARED_FUNCTION: return "Undeclared function";
                case SEMANTIC_ERROR_VARIABLE_ALREADY_DECLARED: return "Variable already declared";
                case SEMANTIC_ERROR_INVALID_OPERATION:      return "Invalid operation";
                case SEMANTIC_ERROR_INVALID_FUNCTION_CALL:  return "Invalid function call";
                case SEMANTIC_ERROR_INVALID_ARGUMENT_COUNT: return "Invalid argument count";
                case SEMANTIC_ERROR_INVALID_ARGUMENT_TYPE:  return "Invalid argument type";
                case SEMANTIC_ERROR_INVALID_RETURN_TYPE:    return "Invalid return type";
                case SEMANTIC_ERROR_NOT_A_FUNCTION:         return "Not a function";
                case SEMANTIC_ERROR_BREAK_OUTSIDE_LOOP:     return "Break outside loop";
                case SEMANTIC_ERROR_UNIMPLEMENTED_NODE:     return "Unimplemented node";
                case SEMANTIC_ERROR_CONTINUE_OUTSIDE_LOOP:  return "Continue outside loop";
                case SEMANTIC_ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER: return "Variable has no type or initializer";
                default: return "Unknown semantic error";
            }
        default: return "Unknown error type";
    }
}
