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

static Token handle_oper(Lexer *lexer);
static Token handle_paren(Lexer *lexer);
static Token handle_num(Lexer *lexer);
static Token handle_ident(Lexer *lexer);
static Token handle_str(Lexer *lexer);

static wchar_t* read_ident(Lexer *lexer);
static wchar_t* read_num(Lexer *lexer);
static wchar_t* read_str(Lexer *lexer, wchar_t quote_char);

static size_t input_len = 0;

void read_ch(Lexer *lexer)
{
    if (lexer->nextpos >= input_len){
        lexer->ch = 0;
    }
    else{
        lexer->ch = lexer->input[lexer->nextpos];
    }
    lexer->pos = lexer->nextpos;
    lexer->nextpos++;
    lexer->column++;
}

void skip_space(Lexer *lexer)
{
    while (1){
        switch (lexer->ch){
            case ' ': case '\t': case '\r':
                read_ch(lexer);
                break;

            case '\n':
                read_ch(lexer);
                lexer->line++;
                lexer->column = 1;
                break;

            default: return;
        }
    }
}

wchar_t peek_ch(const Lexer *lexer)
{
    if (lexer->nextpos < input_len){
        return lexer->input[lexer->nextpos];
    }
    return L'\0';
}

Lexer* lex_new(const wchar_t *input)
{
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
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

    return lexer;
}

Token tok_next(Lexer *lexer)
{
    skip_space(lexer);

    wchar_t ch_str[2] = {lexer->ch, L'\0'};
    Token token;

    switch (lexer->ch){
        case L'+': case L'-':
        case L'*': case L'/':
        case L'=': case L'!':
        case L'<': case L'>':
        case L'&': case L'|':
        case L'.': case L',':
        case L':':
            token = handle_oper(lexer);
            break;

        case L'(': case L')':
        case L'{': case L'}':
        case L'[': case L']':
            if (lexer->ch == L'(' || lexer->ch == L'{' || lexer->ch == L'[') {
                lexer->paren_balance++;
            } else if (lexer->ch == L')' || lexer->ch == L'}' || lexer->ch == L']') {
                lexer->paren_balance--;
            }
            token = handle_paren(lexer);
            break;

        case L'"': case L'\'':
            token = handle_str(lexer);
            break;

        case L'\0':
            if (lexer->paren_balance != 0) {
                wprintf(L"[ERROR] Unmatched parentheses detected\n");
                exit(EXIT_FAILURE);
            }
            token = tok_new(TYPE_SERVICE, T_EOF, L"EOF");
            break;

        case '#':
            while (lexer->ch != L'\n' && lexer->ch != L'\0'){
                read_ch(lexer);
            }
            skip_space(lexer);
            if (lexer->ch == L'\n'){
                read_ch(lexer);
                lexer->line++;
                lexer->column = 1;
            }
            token = tok_new(TYPE_SERVICE, T_COMMENT, L"COMMENT");
            break;

        case L'0' ... L'9':
            token = handle_num(lexer);
            break;

        default:
            if (iswalpha(lexer->ch) || lexer->ch == L'_'){
                token = handle_ident(lexer);
            }
            else{
                token = tok_new(TYPE_SERVICE, T_ILLEGAL, ch_str);
            }
            break;
    }
    if(token.tag == TYPE_SERVICE && token.service == T_ILLEGAL){
        wprintf(L"\n%ls", lexer->input);
        wprintf(L"%*c\n", lexer->column, '^');
        wprintf(L"[ERROR] %ls at line %d, column %d\n", token.literal, lexer->line, lexer->column);
        exit(EXIT_FAILURE);
    }
    return token;
}

static const Keyword *find_operator(const wchar_t *op)
{
    for (size_t i = 0; i < operators_count; i++){
        if (wcscmp(op, operators[i].literal) == 0){
            return &operators[i];
        }
    }
    
    return NULL;
}

static Token handle_oper(Lexer *lexer)
{
    wchar_t current = lexer->ch;
    wchar_t next = peek_ch(lexer);

    if (next != L'\0') {
        wchar_t potential_op[3] = {current, next, L'\0'};
        const Keyword *op = find_operator(potential_op);
        if (op) {
            read_ch(lexer);
            read_ch(lexer);
            return tok_new(TYPE_OPERATOR, op->type, op->literal);
        }
    }
    
    read_ch(lexer);
    switch (current){
        case L'+': return tok_new(TYPE_OPERATOR, T_PLUS, L"+");
        case L'-': return tok_new(TYPE_OPERATOR, T_MINUS, L"-");
        case L'*': return tok_new(TYPE_OPERATOR, T_ASTERISK, L"*");
        case L'/': return tok_new(TYPE_OPERATOR, T_SLASH, L"/");
        case L'=': return tok_new(TYPE_OPERATOR, T_ASSIGN, L"=");
        case L'!': return tok_new(TYPE_OPERATOR, T_NOT, L"!");
        case L'<': return tok_new(TYPE_OPERATOR, T_LANGLE, L"<");
        case L'>': return tok_new(TYPE_OPERATOR, T_RANGLE, L">");
        case L'&': return tok_new(TYPE_OPERATOR, T_AND, L"&");
        case L'|': return tok_new(TYPE_OPERATOR, T_OR, L"|");
        case L'.': return tok_new(TYPE_OPERATOR, T_DOT, L".");
        case L',': return tok_new(TYPE_OPERATOR, T_COMMA, L",");
        case L':': return tok_new(TYPE_OPERATOR, T_COLON, L":");
    }

    wchar_t ch_str[2] = {current, L'\0'};
    return tok_new(TYPE_SERVICE, T_ILLEGAL, ch_str);
}

static Token handle_num(Lexer *lexer)
{
    wchar_t *num_str = read_num(lexer);
    if (!num_str) return tok_new(TYPE_SERVICE, T_ILLEGAL, L"BAD_NUMBER");

    bool is_decimal = wcschr(num_str, L'.') != NULL;
    bool is_hex = wcsstr(num_str, L"0x") == num_str || wcsstr(num_str, L"0X") == num_str;
    bool is_bin = wcsstr(num_str, L"0b") == num_str || wcsstr(num_str, L"0B") == num_str;

    Token token = tok_new(TYPE_VALUE, is_decimal ? T_DECIMAL : T_NUMBER, num_str);
    if (is_hex) token.value = T_HEX;
    else if (is_bin) token.value = T_BIN;

    free(num_str);
    return token;
}

static const Keyword *find_keyword(const wchar_t *ident, const Keyword *keywords, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        if (wcscmp(ident, keywords[i].literal) == 0) {
            return &keywords[i];
        }
    }
    return NULL;
}

static Token handle_ident(Lexer *lexer)
{
    wchar_t *ident = read_ident(lexer);
    if (!ident) return tok_new(TYPE_SERVICE, T_ILLEGAL, L"BAD_IDENT");

    const Keyword *keyword = NULL;
    Token token;

    if ((keyword = find_keyword(ident, keywords, keywords_count))) {
        token = tok_new(TYPE_KEYWORD, keyword->type, ident);
    } else if ((keyword = find_keyword(ident, datatypes, datatypes_count))) {
        token = tok_new(TYPE_DATATYPE, keyword->type, ident);
    } else if ((keyword = find_keyword(ident, modifiers, modifiers_count))) {
        token = tok_new(TYPE_MODIFIER, keyword->type, ident);
    } else {
        token = tok_new(TYPE_SERVICE, T_IDENT, ident);
    }

    free(ident);
    return token;
}

static Token handle_paren(Lexer *lexer)
{
    TParenType type;
    switch (lexer->ch) {
        case L'(': type = T_LPAREN; break;
        case L')': type = T_RPAREN; break;
        case L'{': type = T_LBRACE; break;
        case L'}': type = T_RBRACE; break;
        case L'[': type = T_LBRACKET; break;
        case L']': type = T_RBRACKET; break;
    }
    Token token = tok_new(TYPE_PAREN, type, (wchar_t[]){lexer->ch, L'\0'});
    read_ch(lexer);
    return token;
}

static Token handle_str(Lexer *lexer)
{
    wchar_t quote_char = lexer->ch; // either ' or "
    TDelimiterType opening_delim_type = (quote_char == L'"') ? T_QUOTE : T_SQUOTE;
    (void)tok_new(TYPE_DELIMITER, opening_delim_type, (wchar_t[]){quote_char, L'\0'});
    read_ch(lexer);

    wchar_t *str = read_str(lexer, quote_char);
    if (!str) return tok_new(TYPE_SERVICE, T_ILLEGAL, L"BAD_STRING");

    Token string_token = tok_new(TYPE_VALUE, T_STRING, str);
    free(str);

    if (lexer->ch != quote_char) {
        return tok_new(TYPE_SERVICE, T_ILLEGAL, L"UNCLOSED_STRING");
    }
    (void)tok_new(TYPE_DELIMITER, opening_delim_type, (wchar_t[]){lexer->ch, L'\0'});
    read_ch(lexer);

    return string_token;
}

static wchar_t *read_ident(Lexer *lexer)
{
    wchar_t stack_buffer[IDENT_SIZE];
    wchar_t *buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    while(iswalnum(lexer->ch) || lexer->ch == L'_'){
        if(length >= MAX_IDENT_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            return NULL;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            wchar_t *new_buf = (wchar_t *)realloc(buffer == stack_buffer ? NULL : buffer,
                                                  capacity * sizeof(wchar_t));
            if(!new_buf){
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }

            if(buffer == stack_buffer){
                memcpy(new_buf, stack_buffer, length * sizeof(wchar_t));
            }
            
            buffer = new_buf;
        }

        buffer[length++] = lexer->ch;
        read_ch(lexer);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        wchar_t *heap_copy = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
        if (!heap_copy) return NULL;

        wcscpy(heap_copy, buffer);
        return heap_copy;
    }
    return buffer;
}

static wchar_t *read_num(Lexer *lexer)
{
    wchar_t stack_buffer[NUM_SIZE];
    wchar_t *buffer = stack_buffer;
    size_t capacity = NUM_SIZE;
    size_t length = 0;

    bool seen_decimal_point = false;
    bool parsing_hex = false;
    bool parsing_bin = false;

    if (lexer->ch == L'0') {
        wchar_t next = peek_ch(lexer);
        if (next == L'x' || next == L'X') {
            parsing_hex = true;
        } else if (next == L'b' || next == L'B') {
            parsing_bin = true;
        }
    }

    while (1) {
        wchar_t ch = lexer->ch;

        bool accept = false;
        if (parsing_hex) {
            accept = iswdigit(ch) || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F') || ch == L'x' || ch == L'X';
        } 
        else if (parsing_bin) {
            accept = (ch == L'0' || ch == L'1') || ch == L'b' || ch == L'B';
        } 
        else {
            // Decimal or float
            if (iswdigit(ch)) accept = true;
            else if (ch == L'.' && !seen_decimal_point) {
                accept = true;
                seen_decimal_point = true;
            }
        }

        if (!accept) break;

        if (length >= MAX_NUM_LENGTH){
            if (buffer != stack_buffer) free(buffer);
            return NULL;
        }
        if (length >= capacity - 1){
            size_t new_capacity = capacity * 2;
            wchar_t *new_buf = (wchar_t *)realloc(buffer == stack_buffer ? NULL : buffer,
                                                  new_capacity * sizeof(wchar_t));
            if (!new_buf){
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }
            if (buffer == stack_buffer) {
                wmemcpy(new_buf, stack_buffer, length);
            }
            buffer = new_buf;
            capacity = new_capacity;
        }

        buffer[length++] = ch;
        read_ch(lexer);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer){
        wchar_t *heap_copy = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
        if (!heap_copy) return NULL;
        wmemcpy(heap_copy, buffer, length + 1);
        return heap_copy;
    }

    if (length < capacity / 2){
        wchar_t *shrunk_buf = (wchar_t *)realloc(buffer, (length + 1) * sizeof(wchar_t));
        if (shrunk_buf) buffer = shrunk_buf;
    }
    return buffer;
}

static wchar_t *read_str(Lexer *lexer, wchar_t quote_char)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    wchar_t *buffer = malloc(capacity * sizeof(wchar_t));
    if (!buffer) return NULL;

    while (lexer->ch != quote_char && lexer->ch != L'\0') {
        if (length >= MAX_STR_LENGTH) {
            free(buffer);
            return NULL;
        }

        if (lexer->ch == L'\\') {
            read_ch(lexer);
            switch (lexer->ch) {
            case L'n': buffer[length++] = L'\n'; break;
            case L't': buffer[length++] = L'\t'; break;
            case L'r': buffer[length++] = L'\r'; break;
            case L'"': buffer[length++] = L'\"'; break;
            case L'\'': buffer[length++] = L'\''; break;
            case L'\\': buffer[length++] = L'\\'; break;
            case L'0': buffer[length++] = L'\0'; break;
            default:
                free(buffer);
                return NULL;
            }
            read_ch(lexer);
            continue;
        }

        if (length >= capacity - 1) {
            capacity *= 2;
            wchar_t *new_buf = realloc(buffer, capacity * sizeof(wchar_t));
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }

        buffer[length++] = lexer->ch;
        read_ch(lexer);
    }

    if (lexer->ch != quote_char) {
        free(buffer);
        return NULL;
    }

    buffer[length] = L'\0';

    if (length < capacity / 2) {
        wchar_t *shrunk_buf = realloc(buffer, (length + 1) * sizeof(wchar_t));
        if (shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

void lex_free(Lexer *lexer)
{
    if (lexer){
        free(lexer->input);
        free(lexer);
    }
}