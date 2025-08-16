#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>
#include <stdio.h>

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

static wchar_t* read_ident(struct lexer* lex);
static wchar_t* read_num(struct lexer* lex);
static wchar_t* read_str(struct lexer* lex, wchar_t quote_char);

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

wchar_t peek_ch(const struct lexer* lex)
{
    if (lex->nextpos < input_len){
        return lex->input[lex->nextpos];
    }
    return L'\0';
}

struct lexer* new_lexer(const wchar_t* input)
{
    struct lexer* lexer = (struct lexer*)malloc(sizeof(struct lexer));
    if (!lexer) return NULL;

    input_len = wcslen(input);
    lexer->input = (wchar_t*)malloc((input_len + 1) * sizeof(wchar_t));
    if (!lexer->input){
        free(lexer);
        return NULL;
    }

    wcscpy(lexer->input, input);
    lexer->input[input_len] = L'\0';

    lexer->pos = 0;
    lexer->nextpos = 1;
    lexer->ch = input[0];
    lexer->line = 1;
    lexer->column = 1;
    lexer->paren_balance = 0;

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

    wchar_t ch_str[2] = {lex->ch, L'\0'};
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
                    struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_UNMATCHED_PAREN, 
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
                struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_UNMATCHED_PAREN, 
                                        lex->line, lex->column, 1, lex->input);
                new_lexer_error(lex, err);
            }
            token = new_token(CATEGORY_SERVICE, SERV_EOF, L"EOF");
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
            token = new_token(CATEGORY_SERVICE, SERV_COMMENT, L"COMMENT");
            break;

        default:
            if (iswalnum(lex->ch) || lex->ch == L'_'){
                token = handle_ident(lex);
            } else {
                token = new_token(CATEGORY_SERVICE, SERV_ILLEGAL, ch_str);
                struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_ILLEGAL_CHARACTER,
                                        lex->line, lex->column, wcslen(ch_str), lex->input);
                read_ch(lex);
            }
            break;
    }
    if(token.category == CATEGORY_SERVICE && token.service == SERV_ILLEGAL){
        
    }
    return token;
}

static const struct keyword* find_oper(const wchar_t* op)
{
    for (size_t i = 0; i < operators_count; i++){
        if (wcscmp(op, operators[i].literal) == 0) return &operators[i];
    }
    return NULL;
}

static struct token handle_oper(struct lexer* lex)
{
    wchar_t current = lex->ch;
    wchar_t next = peek_ch(lex);

    if (next != L'\0') {
        wchar_t potential_op[3] = {current, next, L'\0'};
        const struct keyword *op = find_oper(potential_op);
        if (op) {
            read_ch(lex);
            read_ch(lex);
            return new_token(CATEGORY_OPERATOR, op->type, op->literal);
        }
    }
    
    read_ch(lex);
    switch (current){
        case L'+': return new_token(CATEGORY_OPERATOR, OP_PLUS,     L"+");
        case L'-': return new_token(CATEGORY_OPERATOR, OP_MINUS,    L"-");
        case L'*': return new_token(CATEGORY_OPERATOR, OP_ASTERISK, L"*");
        case L'/': return new_token(CATEGORY_OPERATOR, OP_SLASH,    L"/");
        case L'=': return new_token(CATEGORY_OPERATOR, OP_ASSIGN,   L"=");
        case L'!': return new_token(CATEGORY_OPERATOR, OP_NOT,      L"!");
        case L'<': return new_token(CATEGORY_OPERATOR, OP_LANGLE,   L"<");
        case L'>': return new_token(CATEGORY_OPERATOR, OP_RANGLE,   L">");
        case L'.': return new_token(CATEGORY_OPERATOR, OP_DOT,      L".");
        case L',': return new_token(CATEGORY_OPERATOR, OP_COMMA,    L",");
        case L':': return new_token(CATEGORY_OPERATOR, OP_COLON,    L":");
    }

    return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, (wchar_t[]){current, L'\0'});
}

static const struct keyword* find_keyword(const wchar_t* ident, const struct keyword* keywords, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        if (wcscmp(ident, keywords[i].literal) == 0) {
            return &keywords[i];
        }
    }
    return NULL;
}

static struct token handle_ident(struct lexer* lex)
{
    struct token tok;

    if(iswdigit(lex->ch)){
        wchar_t *num = read_num(lex);
        if (num) {
            tok =  new_token(CATEGORY_SERVICE, VAL_NUMBER, num);
            free(num); return tok;
        }
    }

    wchar_t *ident = read_ident(lex);
    if (!ident) {
        free(ident);
        for(int i = 0; iswalnum(lex->ch); ++i) read_ch(lex);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, L"INVALID_IDENT");
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
    wchar_t* num_str = read_num(lex);
    if (!num_str) return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, L"BAD_NUMBER");

    bool is_decimal = wcschr(num_str, L'.') != NULL;
    bool is_hex = wcsstr(num_str, L"0x") == num_str || wcsstr(num_str, L"0X") == num_str;
    bool is_bin = wcsstr(num_str, L"0b") == num_str || wcsstr(num_str, L"0B") == num_str;

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
    struct token token = new_token(CATEGORY_PAREN, type, (wchar_t[]){lex->ch, L'\0'});
    read_ch(lex);
    return token;
}

static struct token handle_str(struct lexer* lex)
{
    wchar_t quote_char = lex->ch;
    enum delimiter_category opening_delim_type = (quote_char == L'"') ? DELIM_QUOTE : DELIM_SQUOTE;
    (void)new_token(CATEGORY_DELIMITER, opening_delim_type, (wchar_t[]){quote_char, L'\0'});
    read_ch(lex);

    wchar_t* str = read_str(lex, quote_char);
    if (!str) {
        struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_STRING, 
                                lex->line, lex->column, 1, lex->input);
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, L"INVALID_STRING");
    }
    if (lex->ch != quote_char) {
        free(str);
        struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_UNCLOSED_STRING, 
                                lex->line, lex->column, 1, lex->input);
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, L"UNCLOSED_STRING");
    }

    struct token string_token = new_token(CATEGORY_VALUE, VAL_STRING, str);
    free(str);

    (void)new_token(CATEGORY_DELIMITER, opening_delim_type, (wchar_t[]){lex->ch, L'\0'});
    read_ch(lex);

    return string_token;
}

static wchar_t* read_ident(struct lexer* lex)
{
    wchar_t stack_buffer[IDENT_SIZE];
    wchar_t* buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    while(iswalnum(lex->ch) || lex->ch == L'_'){
        if(length >= MAX_IDENT_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                    lex->line, lex->column, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }
        if(!iswalpha(lex->ch)){
            for(int i = 0; !iswalpha(lex->ch); ++i){
                read_ch(lex);
                length++;
            }
            struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                    lex->line, lex->column - length, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            wchar_t* new_buf = (wchar_t*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(wchar_t));
            if(!new_buf){
                if (buffer != stack_buffer) free(buffer);
                if (length >= MAX_IDENT_LENGTH) {
                    struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_IDENTIFIER, 
                                            lex->line, lex->column, length, lex->input);
                    new_lexer_error(lex, err);
                }
                return NULL;
            }

            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(wchar_t));
            
            buffer = new_buf;
        }

        buffer[length++] = lex->ch;
        read_ch(lex);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        wchar_t* heap_copy = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
        if (!heap_copy) return NULL;

        wcscpy(heap_copy, buffer);
        return heap_copy;
    }
    return buffer;
}

static wchar_t* read_num(struct lexer* lex)
{
    wchar_t stack_buffer[NUM_SIZE];
    wchar_t* buffer = stack_buffer;
    size_t capacity = NUM_SIZE;
    size_t length = 0;

    bool is_decimal = false;
    bool is_hex = false;
    bool is_bin = false;

    if (lex->ch == L'0') {
        wchar_t next = peek_ch(lex);
        if (next == L'x' || next == L'X') {
            is_hex = true;
        }
        else if (next == L'b' || next == L'B') {
            is_bin = true;
        }
    }

    while (1) {
        wchar_t ch = lex->ch;

        bool accept = false;
        if (is_hex) {
            accept = iswdigit(ch) || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F') || ch == L'x' || ch == L'X';
        } 
        else if (is_bin) {
            accept = (ch == L'0' || ch == L'1') || ch == L'b' || ch == L'B';
        } 
        else {
            if (iswdigit(ch)) accept = true;
            else if (ch == L'.' && !is_decimal) {
                accept = true;
                is_decimal = true;
            }
        }

        if (length > 0 && (iswalpha(lex->ch) || lex->ch == L'_')) {
            if (buffer != stack_buffer) free(buffer);
            lex->pos -= length;
            lex->nextpos = lex->pos + 1;
            lex->column -= length;
            return NULL;
        }

        if (!accept) break;

        if (length >= MAX_NUM_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_NUMBER, 
                                    lex->line, lex->column, length, lex->input);
            new_lexer_error(lex, err);
            return NULL;
        }
        if (length >= capacity - 1){
            size_t new_capacity = capacity * 2;
            wchar_t* new_buf = (wchar_t*)realloc(buffer == stack_buffer ? NULL : buffer,
                                                  new_capacity * sizeof(wchar_t));
            if (!new_buf){
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }
            if (buffer == stack_buffer) wmemcpy(new_buf, stack_buffer, length);

            buffer = new_buf;
            capacity = new_capacity;
        }

        buffer[length++] = ch;
        read_ch(lex);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        wchar_t* heap_copy = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
        if (!heap_copy) return NULL;
        wmemcpy(heap_copy, buffer, length + 1);
        return heap_copy;
    }

    if (length < capacity / 2){
        wchar_t* shrunk_buf = (wchar_t*)realloc(buffer, (length + 1) * sizeof(wchar_t));
        if (shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

static wchar_t* read_str(struct lexer *lex, wchar_t quote_char)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    wchar_t* buffer = (wchar_t*)malloc(capacity * sizeof(wchar_t));
    if (!buffer) return NULL;

    while (lex->ch != quote_char && lex->ch != L'\0') {
        if (length >= MAX_STR_LENGTH) {
            free(buffer);
            struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_INVALID_STRING, 
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
                    struct error* err = new_error(ERROR_SEVERITY_TYPE, ERROR_TYPE_LEXER, LEXER_ERROR_ILLEGAL_CHARACTER, 
                                            lex->line, lex->column, length, lex->input);
                    new_lexer_error(lex, err);
                    return NULL;
            }
            read_ch(lex);
            continue;
        }

        if (length >= capacity - 1) {
            capacity *= 2;
            wchar_t* new_buf = (wchar_t*)realloc(buffer, capacity * sizeof(wchar_t));
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
        wchar_t *shrunk_buf = (wchar_t*)realloc(buffer, (length + 1) * sizeof(wchar_t));
        if (shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

void free_lexer(struct lexer* lex)
{
    if (lex){
        if(lex->errors) {
            for(int i = 0; i < lex->errors_count; i++) {
                free_error(lex->errors[i]);
            }
            lex->errors = NULL;
        }
        free(lex->input);
        free(lex);
    }
}