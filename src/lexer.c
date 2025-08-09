#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>

#include "lexer.h"

#define DEBUG

#ifdef DEBUG
#include <stdio.h>

const char *token_type_to_str(T_TypeTag tag){
    static const char *names[] = {
        "SERVICE", "OPERATOR", "KEYWORD", "PAREN",
        "DELIMITER", "DATATYPE", "VALUE", "MODIFIER"};
    return (tag < sizeof(names) / sizeof(names[0])) ? names[tag] : "UNKNOWN";
}

#define print_token(t)                         \
wprintf(L"%s(%ls)\n", \
    token_type_to_str((t)->tag), (t)->literal)
#else
#define print_token(t)
#endif

#define NUM_SIZE 32
#define MAX_NUM_LENGTH 128

#define IDENT_SIZE 16
#define MAX_IDENT_LENGTH 64

#define STR_SIZE 64
#define MAX_STR_LENGTH 4096

Lexer lexer = {.input = NULL, .pos = 0, .nextpos = 1, .ch = 0};

const Keyword operators[] = {
    {L"++", T_INCREM},  {L"--", T_DECREM},
    {L"==", T_EQ},      {L"!=", T_NEQ},
    {L"+=", T_ADD},     {L"-=", T_SUB},
    {L"*=", T_MUL},     {L"/=", T_DIV},
    {L"&&", T_AND},     {L"||", T_OR},
    {L"<=", T_LTE},     {L">=", T_GTE},
    {L"..", T_RANGE},   {L"->", T_RETTYPE},
};

const Keyword keywords[] = {
    {L"if", T_IF},          {L"else", T_ELSE},
    {L"elif", T_ELIF},      {L"for", T_FOR},
    {L"do", T_DO},          {L"while", T_WHILE},
    {L"func", T_FUNC},      {L"return", T_RETURN},
    {L"break", T_BREAK},    {L"continue", T_CONTINUE},
    {L"default", T_DEFAULT},{L"match", T_MATCH},
    {L"case", T_CASE},      {L"struct", T_STRUCT},
    {L"enum", T_ENUM},      {L"union", T_UNION},
    {L"import", T_IMPORT},  {L"type", T_TYPE},
    {L"trait", T_TRAIT},    {L"try", T_TRY},
    {L"catch", T_CATCH},    {L"async", T_ASYNC},
    {L"await", T_AWAIT},    {L"test", T_TEST},
    {L"assert", T_ASSERT},  {L"verify", T_VERIFY},
    {L"where", T_WHERE},    {L"rollback", T_ROLLBACK},
    {L"commit", T_COMMIT},  {L"fork", T_FORK},
    {L"merge", T_MERGE},    {L"revert", T_REVERT},
    {L"push", T_PUSH},      {L"pull", T_PULL},
    {L"clone", T_CLONE},    {L"simulate", T_SIMULATE},
    {L"choose", T_CHOOSE},  {L"timeline", T_TIMELINE},  
};

const Keyword datatypes[] = {
    /* basic types */
    {L"int", T_INT},        {L"uint", T_UINT},
    {L"float", T_FLOAT},    {L"str", T_STR},
    {L"bool", T_BOOL},      {L"void", T_VOID},
    {L"uni", T_UNI},        {L"tensor", T_TENSOR},

    /* exact types */
    {L"int8", T_INT8},      {L"in16", T_INT16},
    {L"int32", T_INT32},    {L"int64", T_INT64},
    {L"uint8", T_UINT8},    {L"uint16", T_UINT16},
    {L"uint32", T_UINT32},  {L"uint64", T_UINT64},
    {L"float32", T_FLOAT32},{L"float64", T_FLOAT64},
};

const Keyword modifiers[] = {
    {L"var", T_VAR},        {L"const", T_CONST},
    {L"final", T_FINAL},    {L"static", T_STATIC},
    {L"event", T_EVENT},    {L"signal", T_SIGNAL},
    {L"solve", T_SOLVE},    {L"snapshot", T_SNAPSHOT},
};

const size_t operators_count = sizeof(operators) / sizeof(Keyword);
const size_t keywords_count = sizeof(keywords) / sizeof(Keyword);
const size_t datatype_count = sizeof(datatypes) / sizeof(Keyword);
const size_t modifiers_count = sizeof(modifiers) / sizeof(Keyword);

void read_ch(Lexer *lexer)
{
    if (lexer->nextpos >= wcslen(lexer->input)){
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

Lexer* lex_new(const wchar_t *input)
{
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    size_t len = wcslen(input);
    lexer->input = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    if (!lexer->input){
        free(lexer);
        return NULL;
    }

    wcscpy(lexer->input, input);
    lexer->input[len] = L'\0';

    lexer->pos = 0;
    lexer->nextpos = 1;
    lexer->ch = input[0];
    lexer->line = 1;
    lexer->column = 1;

    return lexer;
}

wchar_t peek_ch(const Lexer *lexer)
{
    if (lexer->nextpos < wcslen(lexer->input)){
        return lexer->input[lexer->nextpos];
    }
    return L'\0';
}

int peek_exp_ch(const Lexer *lexer, char exp_ch)
{
    if (lexer->input[lexer->nextpos] == exp_ch){
        return 0;
    }
    else{
        return -1;
    }
}

Token tok_new(const T_TypeTag tag, const int lit_tag, const wchar_t *literal)
{
    Token token = {
        .tag = tag,
        .literal = NULL};

    if (literal){
        token.literal = wcsdup(literal);
        if (!token.literal){
            token.tag = TYPE_SERVICE;
            token.service = T_ILLEGAL;
            return token;
        }
    }

    switch (tag){
        case TYPE_SERVICE:    token.service =    (TServiceType)lit_tag; break;
        case TYPE_OPERATOR:   token.oper =       (TOperatorType)lit_tag; break;
        case TYPE_KEYWORD:    token.keyword =    (TKeywordType)lit_tag; break;
        case TYPE_PAREN:      token.paren =      (TParenType)lit_tag; break;
        case TYPE_DELIMITER:  token.delim =      (TDelimiterType)lit_tag; break;
        case TYPE_DATATYPE:   token.dtype =      (TDataType)lit_tag; break;
        case TYPE_VALUE:      token.value =      (TValueType)lit_tag; break;
        case TYPE_MODIFIER:   token.modifier =   (TModifierType)lit_tag; break;
    }

#ifdef DEBUG
    print_token(&token);
#endif

    return token;
}

static int paren_balance = 0;

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
                paren_balance++;
            } else if (lexer->ch == L')' || lexer->ch == L'}' || lexer->ch == L']') {
                paren_balance--;
            }
            token = handle_paren(lexer);
            break;

        case L'"': case L'\'':
            token = handle_str(lexer);
            break;

        case L'\0':
            if (paren_balance != 0) {
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

    Token token = tok_new(TYPE_VALUE, T_NUMBER, num_str);

    if (wcschr(num_str, L'.') != NULL) token.oper = T_DECIMAL;
    if (wcschr(num_str, L'x') != NULL) token.oper = T_HEX;
    else if (wcschr(num_str, L'b') != NULL) token.oper = T_BIN;

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
    } else if ((keyword = find_keyword(ident, datatypes, datatype_count))) {
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
    TDelimiterType opening_delim_type = (lexer->ch == L'"') ? T_QUOTE : T_SQUOTE;
    Token opening_quote_token = tok_new(TYPE_DELIMITER, opening_delim_type, (wchar_t[]){lexer->ch, L'\0'});
    read_ch(lexer);

    wchar_t *str = read_str(lexer);
    if (!str) return tok_new(TYPE_SERVICE, T_ILLEGAL, L"BAD_STRING");

    Token string_token = tok_new(TYPE_VALUE, T_STRING, str);
    free(str);

    Token closing_quote_token = tok_new(TYPE_DELIMITER, opening_delim_type, (wchar_t[]){lexer->ch, L'\0'});
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
    bool has_delim_point = false;

    while (iswdigit(lexer->ch) || wcschr(L".xb", lexer->ch) != NULL) {
        if (wcschr(L".xb", lexer->ch) != NULL) {
            if (has_delim_point) {
                if (buffer != stack_buffer) free(buffer);
                return NULL;
            }
            has_delim_point = true;
        }

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

        buffer[length++] = lexer->ch;
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

static wchar_t *read_str(Lexer *lexer)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    wchar_t *buffer = malloc(capacity * sizeof(wchar_t));
    if (!buffer) return NULL;

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
            case L'"': buffer[length++] = L'\"'; break;
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
    if (lexer){
        free(lexer->input);
        free(lexer);
    }
}

void tok_free(Token *tok)
{
    if (tok){
        free(tok->literal);
        tok->literal = NULL;
    }
}