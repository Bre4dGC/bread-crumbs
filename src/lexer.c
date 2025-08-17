#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include "lexer.h"

#define NUM_SIZE 32
#define MAX_NUM_LENGTH 128

#define IDENT_SIZE 16
#define MAX_IDENT_LENGTH 64

#define STR_SIZE 64
#define MAX_STR_LENGTH 4096

static struct token handle_oper(struct lexer* lex);
static struct token handle_paren(struct lexer* lex);
static struct token handle_num(struct lexer* lex);
static struct token handle_ident(struct lexer* lex);
static struct token handle_str(struct lexer* lex);

static char* read_ident(struct lexer* lex);
static char* read_num(struct lexer* lex);
static char* read_str(struct lexer* lex, char quote_char);

static size_t input_len = 0;

void read_ch(struct lexer *lex)
{
    if (lex->nextpos >= input_len){
        lex->ch = 0;
    }
    else{
        lex->ch = lex->input[lex->nextpos];
    }
    lex->pos = lex->nextpos;
    lex->nextpos++;
    lex->column++;
}

void skip_whitespace(struct lexer *lex)
{
    while (1){
        switch (lex->ch){
            case ' ': case '\t': case '\r':
                read_ch(lex);
                break;

            case '\n':
                read_ch(lex);
                lex->line++;
                lex->column = 1;
                break;

            default: return;
        }
    }
}

char peek_ch(const struct lexer* lex)
{
    if (lex->nextpos < input_len){
        return lex->input[lex->nextpos];
    }
    return L'\0';
}

struct lexer* new_lexer(const char* input)
{
    struct lexer* lexer = (struct lexer*)malloc(sizeof(struct lexer));
    if (!lexer) return NULL;

    input_len = strlen(input);
    lexer->input = (char*)malloc((input_len + 1) * sizeof(char));
    if (!lexer->input){
        free(lexer);
        return NULL;
    }

    strcpy(lexer->input, input);
    lexer->input[input_len] = L'\0';

    lexer->pos = 0;
    lexer->nextpos = 1;
    lexer->ch = input[0];
    lexer->line = 1;
    lexer->column = 1;
    lexer->paren_balance = 0;
    lexer->errors = NULL;
    lexer->errors_count = 0;

    return lexer;
}

void new_lexer_error(struct lexer* lex, struct error* err)
{
    if (!lex || !err) {
        if (err) free_error(err);
        return;
    }

    if (!lex->errors) {
        lex->errors = (struct error**)malloc(sizeof(struct error*));
        if (!lex->errors) {
            free_error(err);
            return;
        }
        lex->errors[0] = err;
        lex->errors_count = 1;
        return;
    }

    struct error** new_errors = (struct error**)realloc(
        lex->errors, 
        (lex->errors_count + 1) * sizeof(struct error*)
    );
    if (!new_errors) {
        free_error(err);
        return;
    }
    
    lex->errors = new_errors;
    lex->errors[lex->errors_count] = err;
    lex->errors_count++;
}


struct token next_token(struct lexer* lex)
{
    skip_whitespace(lex);

    char ch_str[2] = {lex->ch, L'\0'};
    struct token token;

    switch (lex->ch){
        case L'+': case L'-':
        case L'*': case L'/':
        case L'=': case L'!':
        case L'<': case L'>':
        case L'&': case L'|':
        case L'.': case L',':
        case L':':
            token = handle_oper(lex);
            break;

        case L'(': case L')':
        case L'{': case L'}':
        case L'[': case L']':
            if (lex->ch == L'(' || lex->ch == L'{' || lex->ch == L'[') {
                lex->paren_balance++;
            } else if (lex->ch == L')' || lex->ch == L'}' || lex->ch == L']') {
                if (lex->paren_balance == 0) {
                    struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_UNMATCHED_PAREN, 
                                            lex->line, lex->column, 1, lex->input);
                    new_lexer_error(lex, err);
                } 
                else {
                    lex->paren_balance--;
                }
            }
            token = handle_paren(lex);
            break;

        case L'"': case L'\'':
            token = handle_str(lex);
            break;

        case L'\0':
            if (lex->paren_balance != 0) {
                lex->ch = 0;
                lex->pos = lex->nextpos;
                lex->column++;
                lex->line++;
                struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_UNMATCHED_PAREN, 
                                        lex->line, lex->column, 1, lex->input);
                new_lexer_error(lex, err);
            }
            token = new_token(CATEGORY_SERVICE, SERV_EOF, "EOF");
            break;

        case '#':
            while (lex->ch != L'\n' && lex->ch != L'\0'){
                read_ch(lex);
            }
            skip_whitespace(lex);
            if (lex->ch == L'\n'){
                read_ch(lex);
                lex->line++;
                lex->column = 1;
            }
            token = new_token(CATEGORY_SERVICE, SERV_COMMENT, "COMMENT");
            break;

        default:
            if (isalnum(lex->ch) || lex->ch == L'_'){
                token = handle_ident(lex);
            } else {
                token = new_token(CATEGORY_SERVICE, SERV_ILLEGAL, ch_str);
                struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_ILLEGAL_CHARACTER,
                                        lex->line, lex->column, strlen(ch_str), lex->input);
                new_lexer_error(lex, err);
                read_ch(lex);
            }
            break;
    }
    return token;
}

static const struct keyword* find_oper(const char* op)
{
    for (size_t i = 0; i < operators_count; i++){
        if (strcmp(op, operators[i].literal) == 0) return &operators[i];
    }
    return NULL;
}

static struct token handle_oper(struct lexer* lex)
{
    char current = lex->ch;
    char next = peek_ch(lex);

    if (next != L'\0') {
        char potential_op[3] = {current, next, L'\0'};
        const struct keyword *op = find_oper(potential_op);
        if (op) {
            read_ch(lex);
            read_ch(lex);
            return new_token(CATEGORY_OPERATOR, op->type, op->literal);
        }
    }
    
    read_ch(lex);
    switch (current){
        case L'+': return new_token(CATEGORY_OPERATOR, OP_PLUS,     "+");
        case L'-': return new_token(CATEGORY_OPERATOR, OP_MINUS,    "-");
        case L'*': return new_token(CATEGORY_OPERATOR, OP_ASTERISK, "*");
        case L'/': return new_token(CATEGORY_OPERATOR, OP_SLASH,    "/");
        case L'=': return new_token(CATEGORY_OPERATOR, OP_ASSIGN,   "=");
        case L'!': return new_token(CATEGORY_OPERATOR, OP_NOT,      "!");
        case L'<': return new_token(CATEGORY_OPERATOR, OP_LANGLE,   "<");
        case L'>': return new_token(CATEGORY_OPERATOR, OP_RANGLE,   ">");
        case L'.': return new_token(CATEGORY_OPERATOR, OP_DOT,      ".");
        case L',': return new_token(CATEGORY_OPERATOR, OP_COMMA,    ",");
        case L':': return new_token(CATEGORY_OPERATOR, OP_COLON,    ":");
    }

    return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, (char[]){current, L'\0'});
}

static const struct keyword* find_keyword(const char* ident, const struct keyword* keywords, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(ident, keywords[i].literal) == 0) {
            return &keywords[i];
        }
    }
    return NULL;
}

static struct token handle_ident(struct lexer* lex)
{
    struct token tok;

    if(isdigit(lex->ch)){
        char *num = read_num(lex);
        if (num) {
            tok =  new_token(CATEGORY_VALUE, VAL_NUMBER, num);
            free(num); return tok;
        }
    }

    char *ident = read_ident(lex);
    if (!ident) {
        free(ident);
        for(int i = 0; isalnum(lex->ch); ++i) read_ch(lex);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "INVALID_IDENT");
    }

    const struct keyword *kw = find_keyword(ident, keywords, keywords_count);

    if (kw) {
        tok = new_token(kw->category, kw->type, ident);
    }
    else {
        tok = new_token(CATEGORY_SERVICE, SERV_IDENT, ident);
    }

    free(ident);
    return tok;
}

static struct token handle_num(struct lexer *lex)
{
    char* num_str = read_num(lex);
    if (!num_str) return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "BAD_NUMBER");

    bool is_decimal = strchr(num_str, L'.') != NULL;
    bool is_hex = strstr(num_str, "0x") == num_str || strstr(num_str, "0X") == num_str;
    bool is_bin = strstr(num_str, "0b") == num_str || strstr(num_str, "0B") == num_str;

    struct token token = new_token(CATEGORY_VALUE, is_decimal ? VAL_DECIMAL : VAL_NUMBER, num_str);
    if (is_hex) token.value = VAL_HEX;
    else if (is_bin) token.value = VAL_BIN;

    free(num_str);
    return token;
}

static struct token handle_paren(struct lexer *lex)
{
    enum paren_category type;
    switch (lex->ch) {
        case L'(': type = PAR_LPAREN; break;
        case L')': type = PAR_RPAREN; break;
        case L'{': type = PAR_LBRACE; break;
        case L'}': type = PAR_RBRACE; break;
        case L'[': type = PAR_LBRACKET; break;
        case L']': type = PAR_RBRACKET; break;
    }
    struct token token = new_token(CATEGORY_PAREN, type, (char[]){lex->ch, L'\0'});
    read_ch(lex);
    return token;
}

static struct token handle_str(struct lexer* lex)
{
    char quote_char = lex->ch;
    enum delimiter_category opening_delim_type = (quote_char == L'"') ? DELIM_QUOTE : DELIM_SQUOTE;
    struct token opening_delim = new_token(CATEGORY_DELIMITER, opening_delim_type, (char[]){quote_char, L'\0'});
    free_token(&opening_delim);
    read_ch(lex);

    char* str = read_str(lex, quote_char);
    if (!str) {
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_STRING, 
                                lex->line, lex->column, 1, lex->input);
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "INVALID_STRING");
    }
    if (lex->ch != quote_char) {
        free(str);
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_UNCLOSED_STRING, 
                                lex->line, lex->column, 1, lex->input);
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "UNCLOSED_STRING");
    }

    struct token string_token = new_token(CATEGORY_VALUE, VAL_STRING, str);
    free(str);

    struct token closing_delim = new_token(CATEGORY_DELIMITER, opening_delim_type, (char[]){lex->ch, L'\0'});
    free_token(&closing_delim);
    read_ch(lex);

    return string_token;
}

static char* read_ident(struct lexer* lex)
{
    char stack_buffer[IDENT_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    while(isalnum(lex->ch) || lex->ch == L'_'){
        if(length >= MAX_IDENT_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                    lex->line, lex->column, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }
        if(!isalpha(lex->ch)){
            for(int i = 0; !isalpha(lex->ch); ++i){
                read_ch(lex);
                length++;
            }
            struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                    lex->line, lex->column - length, length, lex->input);
            new_lexer_error(lex, err);
            if (buffer != stack_buffer) free(buffer);
            return NULL;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
            if(!new_buf){
                if (buffer != stack_buffer) free(buffer);
                if (length >= MAX_IDENT_LENGTH) {
                    struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                            lex->line, lex->column, length, lex->input);
                    new_lexer_error(lex, err);
                }
                return NULL;
            }

            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
            
            buffer = new_buf;
        }

        buffer[length++] = lex->ch;
        read_ch(lex);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        char* heap_copy = (char*)malloc((length + 1) * sizeof(char));
        if (!heap_copy) return NULL;

        strcpy(heap_copy, buffer);
        return heap_copy;
    }
    return buffer;
}

static char* read_num(struct lexer* lex)
{
    if(!isdigit(lex->ch)) return NULL;

    char stack_buffer[NUM_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = NUM_SIZE;
    size_t length = 0;

    bool is_decimal = false;
    bool is_hex = false;
    bool is_bin = false;

    if (lex->ch == L'0') {
        char next = peek_ch(lex);
        if (next == L'x' || next == L'X') {
            is_hex = true;
        }
        else if (next == L'b' || next == L'B') {
            is_bin = true;
        }
    }

    while (1) {
        char ch = lex->ch;

        bool accept = false;
        if (is_hex) {
            accept = isdigit(ch) || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F') || ch == L'x' || ch == L'X';
        } 
        else if (is_bin) {
            accept = (ch == L'0' || ch == L'1') || ch == L'b' || ch == L'B';
        } 
        else {
            if (isdigit(ch)) accept = true;
            else if (ch == L'.' && !is_decimal) {
                accept = true;
                is_decimal = true;
            }
        }

        if (length > 0 && (isalpha(ch) || ch == L'_') && (!is_hex && !is_bin && !is_decimal)) {
            if (buffer != stack_buffer) free(buffer);
            lex->pos -= length;
            lex->nextpos = lex->pos + 1;
            lex->column -= length;
            return NULL;
        }

        if(!accept) break;
        
        /* TODO: fix */
        // if (!accept && (is_hex || is_bin || is_decimal)){
        //     length -= 2;
        //     for(int i = 0; isalnum(ch); ++i){
        //         read_ch(lex);
        //         length++;
        //     }
        //     struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_NUMBER, 
        //                             lex->line, lex->column - length, length, lex->input);
        //     new_lexer_error(lex, err);
        //     if (buffer != stack_buffer) free(buffer);
        //     return NULL;
        // }

        if (length >= MAX_NUM_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_NUMBER, 
                                    lex->line, lex->column, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }
        if (length >= capacity - 1){
            size_t new_capacity = capacity * 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer,
                                                  new_capacity * sizeof(char));
            if (!new_buf){
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }
            if (buffer == stack_buffer) memcpy(new_buf, stack_buffer, length);

            buffer = new_buf;
            capacity = new_capacity;
        }

        buffer[length++] = ch;
        read_ch(lex);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        char* heap_copy = (char*)malloc((length + 1) * sizeof(char));
        if (!heap_copy) return NULL;
        memcpy(heap_copy, buffer, length + 1);
        return heap_copy;
    }

    if (length < capacity / 2){
        char* shrunk_buf = (char*)realloc(buffer, (length + 1) * sizeof(char));
        if (shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

static char* read_str(struct lexer *lex, char quote_char)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    char* buffer = (char*)malloc(capacity * sizeof(char));
    if (!buffer) return NULL;

    while (lex->ch != quote_char && lex->ch != L'\0') {
        if (length >= MAX_STR_LENGTH) {
            free(buffer);
            struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_STRING, 
                                    lex->line, lex->column, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }

        if (lex->ch == L'\\') {
            read_ch(lex);
            switch (lex->ch) {
                case L'n': buffer[length++] = L'\n'; break;
                case L't': buffer[length++] = L'\t'; break;
                case L'r': buffer[length++] = L'\r'; break;
                case L'"': buffer[length++] = L'\"'; break;
                case L'\'':buffer[length++] = L'\''; break;
                case L'\\':buffer[length++] = L'\\'; break;
                case L'0': buffer[length++] = L'\0'; break;
                default:
                    free(buffer);
                    struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_LEXER, LEXER_ERROR_ILLEGAL_CHARACTER, 
                                            lex->line, lex->column, length, lex->input);
                    new_lexer_error(lex, err);
                    return NULL;
            }
            read_ch(lex);
            continue;
        }

        if (length >= capacity - 1) {
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer, capacity * sizeof(char));
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }

        buffer[length++] = lex->ch;
        read_ch(lex);
    }
    
    buffer[length] = L'\0';

    if (length < capacity / 2) {
        char *shrunk_buf = (char*)realloc(buffer, (length + 1) * sizeof(char));
        if (shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

void free_lexer(struct lexer* lex)
{
    if (lex){
        if(lex->errors) {
            for(int i = 0; i < lex->errors_count; i++) {
                if (lex->errors[i]) free_error(lex->errors[i]);
            }
            free(lex->errors);
        }
        free(lex->input);
        free(lex);
    }
}