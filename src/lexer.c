#include <stdio.h> // remove after test

#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>

#include "../include/lexer.h"

#ifdef DEBUG
#define print_token(t) \
    wprintf(L"Token [%d]: %ls (Value: %ls)\n", \
           (t)->tag, token_type_to_str((t)->tag), (t)->literal)

const char* token_type_to_str(T_TypeTag tag) {
    static const char* names[] = {
        "SERVICE", "OPERATOR", "KEYWORD", "PAREN",
        "DELIMITER", "DATATYPE", "MODIFIER", "COLLECTION"
    };
    return (tag < sizeof(names)/sizeof(names[0])) ? names[tag] : "UNKNOWN";
}
#else
#define print_token(t)
#endif

#define NUM_SIZE 32
#define MAX_NUM_LENGTH 128

#define IDENT_SIZE 16
#define MAX_IDENT_LENGTH 64

#define STR_SIZE 64
#define MAX_STR_LENGTH 4096

Lexer lexer = {.input=NULL, .pos=0, .nextpos=1, .ch=0};

const Operator operators[] = {
    {L"++", T_INCREM}, {L"--", T_DECREM},
    {L"+=", T_ADD},    {L"-=", T_SUB},
    {L"*=", T_MUL},    {L"/=", T_DIV},
    {L"&&", T_AND},    {L"||", T_OR},
    {L"==", T_EQ},     {L"!=", T_NEQ},
    {L"<=", T_LTE},    {L">=", T_GTE},
    {L"..", T_RANGE},  {L"->", T_RETTYPE},
};

const Keyword keywords[] = {
    {L"if",       T_IF},
    {L"else",     T_ELSE},
    {L"elif",     T_ELIF},
    {L"for",      T_FOR},
    {L"do",       T_DO},
    {L"while",    T_WHILE},
    {L"func",     T_FUNC},
    {L"return",   T_RETURN},
    {L"break",    T_BREAK},
    {L"continue", T_CONTINUE},
    {L"default",  T_DEFAULT},
    {L"match",    T_MATCH},
    {L"case",     T_CASE},
    {L"struct",   T_STRUCT},
    {L"enum",     T_ENUM},
    {L"union",    T_UNION},
    {L"import",   T_IMPORT},
    {L"type",     T_TYPE},
    {L"trait",    T_TRAIT},
    {L"try",      T_TRY},
    {L"catch",    T_CATCH},
};

const size_t operators_count = sizeof(operators) / sizeof(Keyword);
const size_t keywords_count = sizeof(keywords) / sizeof(Keyword);

void read_ch(Lexer *lexer)
{
    if(lexer->nextpos >= wcslen(lexer->input)) {
        lexer->ch = 0;
    }
    else {
        lexer->ch = lexer->input[lexer->nextpos];
    }
    lexer->pos = lexer->nextpos;
    lexer->nextpos++;
}

void skip_space(Lexer *lexer)
{
    while(1){
        switch(lexer->ch){
            case ' ': case '\t': case '\r':
                read_ch(lexer);
                break;
            case '\n':
                read_ch(lexer);
                lexer->line++;
                break;
            case '#':
                while(lexer->ch != '\n' || lexer->ch != '\0'){
                    read_ch(lexer);
                }
                break;
            default:
                return;
        }
    }
}

Lexer* lex_new(const wchar_t *input)
{
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    size_t len = wcslen(input);
    lexer->input = (wchar_t *)malloc(len + 1);
    if (!lexer->input) {
        free(lexer);
        return NULL;
    }

    wcscpy(lexer->input, input);

    lexer->pos = 0;
    lexer->nextpos = 1;
    lexer->ch = input[0];
    lexer->line = 1;
    lexer->column = 1;

    return lexer;
}

wchar_t peek_ch(const Lexer *lexer)
{
    if (lexer->nextpos < wcslen(lexer->input)) {
        return lexer->input[lexer->nextpos];
    }
    return L'\0';
}

int peek_exp_ch(const Lexer *lexer, char exp_ch)
{
    if(lexer->input[lexer->nextpos] == exp_ch) {
        return 0;
    }
    else {
        return -1;
    }
}

Token tok_new(const T_TypeTag tag, const int lit_tag, const wchar_t *literal)
{
    Token token = {
        .tag = tag,
        .literal = NULL
    };

    if (literal) {
        token.literal = wcsdup(literal);
        if (!token.literal) {
            token.tag = TYPE_SERVICE;
            token.service = T_ILLEGAL;
            return token;
        }
    }

    switch(tag) {
        case TYPE_SERVICE:   token.service  = (TServiceType)lit_tag;    break;
        case TYPE_OPERATOR:  token.oper     = (TOperatorType)lit_tag;   break;
        case TYPE_KEYWORD:   token.keyword  = (TKeywordType)lit_tag;    break;
        case TYPE_PAREN:     token.paren    = (TParenType)lit_tag;      break;
        case TYPE_DELIMITER: token.delim    = (TDelimiterType)lit_tag;  break;
        case TYPE_DATATYPE:  token.dtype    = (TDataType)lit_tag;       break;
        case TYPE_MODIFIER:  token.modifier = (TModifierType)lit_tag;   break;
        case TYPE_COLLECTION: token.collection = (TCollectionType)lit_tag; break;
    }

    #ifdef DEBUG
    print_token(&token);
    #endif

    return token;
}

Token tok_next(Lexer *lexer)
{
    skip_space(lexer);

    wchar_t ch_str[2] = {lexer->ch, L'\0'};
    Token token;

    switch(lexer->ch) {
        case L'+': case L'-': case L'*': case L'/':
        case L'=': case L'<': case L'>': case L'!':
        case L'&': case L'|': case L'.': case L',':
        case L':':
            token = handle_oper(lexer);
            break;

        case L'(': case L')': case L'{': case L'}':
        case L'[': case L']':
            token = handle_paren(lexer);
            break;

        case L'"': case L'\'':
            token = handle_str(lexer);
            break;

        case L'\0':
            token = tok_new(TYPE_SERVICE, T_EOF, L"EOF");
            break;

        case L'0'...L'9':
            token = handle_num(lexer);
            break;

        default:
            if(iswalpha(lexer->ch) || lexer->ch == L'_'){
                token = handle_ident(lexer);
            }
            else{
                token = tok_new(TYPE_SERVICE, T_ILLEGAL, ch_str);
            }
            break;
    }
    read_ch(lexer);
    return token;
}

static const Operator* find_operator(const wchar_t* op) {
    size_t left = 0;
    size_t right = operators_count - 1;

    while (left <= right) {
        size_t mid = left + (right - left) / 2;
        int cmp = wcscmp(operators[mid].literal, op);

        if (cmp == 0) return &operators[mid];
        if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return NULL;
}

static Token handle_oper(Lexer *lexer)
{
    wchar_t current = lexer->ch;
    wchar_t next = peek_ch(lexer);

    if (next != L'\0') {
        wchar_t potential_op[3] = {current, next, L'\0'};
        const Operator* op = find_operator(potential_op);

        if (op != NULL) {
            read_ch(lexer);
            return tok_new(TYPE_OPERATOR, op->type, op->literal);
        }
    }

    static const struct {
        wchar_t ch;
        TOperatorType type;
    } single_ops[] = {
        {L'+', T_PLUS}, {L'-', T_MINUS}, {L'*', T_ASTERISK},
        {L'/', T_SLASH}, {L'=', T_ASSIGN}, {L'.', T_DOT},
        {L',', T_COMMA}, {L':', T_COLON}, {L'<', T_LANGLE},
        {L'>', T_RANGLE}
    };

    for (size_t i = 0; i < sizeof(single_ops)/sizeof(single_ops[0]); i++) {
        if (current == single_ops[i].ch) {
            wchar_t ch_str[2] = {current, L'\0'};
            return tok_new(TYPE_OPERATOR, single_ops[i].type, ch_str);
        }
    }

    wchar_t ch_str[2] = {current, L'\0'};
    return tok_new(TYPE_SERVICE, T_ILLEGAL, ch_str);
}

static Token handle_num(Lexer *lexer)
{
    // TODO: implement this
}

static Token handle_ident(Lexer *lexer)
{
    // TODO: implement this
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
    return tok_new(TYPE_PAREN, type, (wchar_t[]){lexer->ch, L'\0'});
}

static Token handle_str(Lexer *lexer)
{
    wchar_t quote = lexer->ch;
    wchar_t *str = read_str(lexer);
    if (!str) {
        return tok_new(TYPE_SERVICE, T_ILLEGAL, L"BAD_STRING");
    }

    Token token = tok_new(TYPE_DELIMITER,
                         (quote == L'"') ? T_QUOTE : T_SQUOTE,
                         (quote == L'"') ? L"\""   : L"'");
    free(str);
    return token;
}

static wchar_t* read_ident(Lexer *lexer)
{
    wchar_t stack_buffer[IDENT_SIZE];
    wchar_t* buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    while (iswalnum(lexer->ch) || lexer->ch == L'_') {
        if (length >= MAX_IDENT_LENGTH) {
            if (buffer != stack_buffer) free(buffer);
            return NULL;
        }

        if (length >= capacity - 1) {
            size_t new_capacity = capacity * 2;
            wchar_t* new_buf = (wchar_t*)realloc(buffer == stack_buffer ? NULL : buffer,
                                      new_capacity * sizeof(wchar_t));
            if (!new_buf) {
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }

            if (buffer == stack_buffer) {
                memcpy(new_buf, stack_buffer, length * sizeof(wchar_t));
            }

            buffer = new_buf;
            capacity = new_capacity;
        }

        buffer[length++] = lexer->ch;
        read_ch(lexer);
    }

    buffer[length] = L'\0';

    if (buffer == stack_buffer) {
        wchar_t* heap_copy = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
        if (!heap_copy) return NULL;
        wcscpy(heap_copy, buffer);
        return heap_copy;
    }
    return buffer;
}

static wchar_t* read_num(Lexer *lexer)
{
    wchar_t stack_buffer[NUM_SIZE];
        wchar_t* buffer = stack_buffer;
        size_t capacity = NUM_SIZE;
        size_t length = 0;
        bool has_decimal_point = false;

        while (iswdigit(lexer->ch) || lexer->ch == L'.') {
            if (lexer->ch == L'.') {
                if (has_decimal_point) {
                    if (buffer != stack_buffer) free(buffer);
                    return NULL;
                }
                has_decimal_point = true;
            }

            if (length >= MAX_NUM_LENGTH) {
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }

            if (length >= capacity - 1) {
                size_t new_capacity = capacity * 2;
                wchar_t* new_buf = (wchar_t*)realloc(buffer == stack_buffer ? NULL : buffer,
                                          new_capacity * sizeof(wchar_t));
                if (!new_buf) {
                    if (buffer != stack_buffer) free(buffer);
                    return NULL;
                }

                if (buffer == stack_buffer) {
                    wmemcpy(new_buf, stack_buffer, length);
                }

                buffer = new_buf;
                capacity = new_capacity;
            }

            buffer[length++] = lexer->ch;
            read_ch(lexer);
        }

        if (length == 0 || (length == 1 && has_decimal_point)) {
            if (buffer != stack_buffer) free(buffer);
            return NULL;
        }

        buffer[length] = L'\0';

        if (buffer == stack_buffer) {
            wchar_t* heap_copy = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
            if (!heap_copy) return NULL;
            wmemcpy(heap_copy, buffer, length + 1);
            return heap_copy;
        }

        if (length < capacity / 2) {
            wchar_t* shrunk_buf = (wchar_t*)realloc(buffer, (length + 1) * sizeof(wchar_t));
            if (shrunk_buf) buffer = shrunk_buf;
        }
        return buffer;
}

static wchar_t* read_str(Lexer *lexer)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    wchar_t *buffer = malloc(capacity * sizeof(wchar_t));
    if (!buffer) return NULL;

    read_ch(lexer);

    while (lexer->ch != L'"' && lexer->ch != L'\0') {
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
                case L'"': buffer[length++] = L'"'; break;
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

    if (lexer->ch != L'"') {
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
    if (lexer) {
        free(lexer->input);
        free(lexer);
    }
}

void tok_free(Token *tok)
{
    if(tok) {
        free(tok->literal);
        free(tok);
    }
}
