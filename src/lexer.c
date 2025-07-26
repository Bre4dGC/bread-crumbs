#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include "../include/lexer.h"

Lexer lexer = {.input=NULL, .pos=0, .nextpos=1, .ch=0};

const Keyword operators[] = {
    {"+=", T_ADD},
    {"-=", T_SUB},
    {"*=", T_MUL},
    {"/=", T_DIV},
    {"&&", T_AND},
    {"||", T_OR},
    {"==", T_EQ},
    {"!=", T_NEQ},
    {"<=", T_LTE},
    {">=", T_GTE},
    {"..", T_RANGE},
    {"->", T_RETTYPE}
};

const Keyword keywords[] = {
    {"if",       T_IF},
    {"else",     T_ELSE},
    {"elif",     T_ELIF},
    {"for",      T_FOR},
    {"do",       T_DO},
    {"while",    T_WHILE},
    {"func",     T_FUNC},
    {"return",   T_RETURN},
    {"break",    T_BREAK},
    {"continue", T_CONTINUE},
    {"default",  T_DEFAULT},
    {"match",    T_MATCH},
    {"case",     T_CASE},
    {"struct",   T_STRUCT},
    {"enum",     T_ENUM},
    {"union",    T_UNION},
    {"import",   T_IMPORT},
    {"type",     T_TYPE},
    {"trait",    T_TRAIT},
    {"try",      T_TRY},
    {"catch",    T_CATCH},
};

const size_t operators_count = sizeof(operators) / sizeof(Keyword);
const size_t keywords_count = sizeof(keywords) / sizeof(Keyword);

void read_ch(Lexer *lex)
{
    if(lex->nextpos >= strlen(lex->input)) {
        lex->ch = 0;
    }
    else {
        lex->ch = lex->input[lex->nextpos];
    }
    lex->pos = lex->nextpos;
    lex->nextpos++;
}

void skip_space(Lexer *lex)
{
    while(1){
        switch(lex->ch){
            case ' ': case '\t': case '\r':
                read_ch(lex);
                break;
            case '\n':
                read_ch(lex);
                lex->line++;
                break;
            case '#':
                while(lex->ch != '\n' || lex->ch != '\0'){
                    read_ch(lex);
                }
                break;
            default:
                return;
        }
    }
}

Lexer* lex_new(const char *input)
{
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    size_t len = strlen(input);
    lexer->input = (char*)malloc(len + 1);
    if (!lexer->input) {
        free(lexer);
        return NULL;
    }

    strcpy(lexer->input, input);

    lexer->pos = 0;
    lexer->nextpos = 1;
    lexer->ch = input[0];
    lexer->line = 1;
    lexer->column = 1;

    return lexer;
}

int lex_peekch(const Lexer *lex, char exp_ch)
{
    if(lex->input[lex->nextpos] == exp_ch) {
        return 0;
    }
    else {
        return -1;
    }
}

Token tok_new(const T_TypeTag ttag, const int value, const wchar_t *literal)
{
    Token token = {
        .tag = ttag,
        .literal = wcsdup(literal)
    };

    switch(ttag) {
        case TYPE_SERVICE:
            token.service = (TServiceType)value;
            break;
        case TYPE_OPERATOR:
            token.oper = (TOperatorType)value;
            break;
        case TYPE_KEYWORD:
            token.keyword = (TKeywordType)value;
            break;
        case TYPE_PAREN:
            token.paren = (TParenType)value;
            break;
        case TYPE_DELIMITER:
            token.delim = (TDelimiterType)value;
            break;
        case TYPE_DATATYPE:
            token.dtype = (TDataType)value;
            break;
        case TYPE_MODIFIER:
            token.modifier = (TModifierType)value;
            break;
        case TYPE_COLLECTION:
            token.collection = (TCollectionType)value;
            break;
        default:
            token.service = T_UNKNOWN;
            break;
    }

    return token;
}

Token tok_next(Lexer *lexer)
{
    Token token;
    wchar_t ch_str[2] = {lexer->ch, L'\0'};

    switch(lexer->ch) {
        case L'+': token = tok_new(TYPE_OPERATOR, T_PLUS, ch_str);
            break;
        case L'-': token = tok_new(TYPE_OPERATOR, T_MINUS, ch_str);
            break;
        case L'*': token = tok_new(TYPE_OPERATOR, T_ASTERISK, ch_str);
            break;
        case L'/': token = tok_new(TYPE_OPERATOR, T_SLASH, ch_str);
            break;
        case L'=': token = tok_new(TYPE_OPERATOR, T_ASSIGN, ch_str);
            break;
        case L'.': token = tok_new(TYPE_OPERATOR, T_DOT, ch_str);
            break;
        case L',': token = tok_new(TYPE_OPERATOR, T_COMMA, ch_str);
            break;
        case L':': token = tok_new(TYPE_OPERATOR, T_COLON, ch_str);
            break;
        case L'<': token = tok_new(TYPE_OPERATOR, T_LANGLE, ch_str);
            break;
        case L'>': token = tok_new(TYPE_OPERATOR, T_RANGLE, ch_str);
            break;
        case L'(': token = tok_new(TYPE_PAREN, T_LROUND, ch_str);
            break;
        case L')': token = tok_new(TYPE_OPERATOR, T_RROUND, ch_str);
            break;
        case L'{': token = tok_new(TYPE_OPERATOR, T_LCURLY, ch_str);
            break;
        case L'}': token = tok_new(TYPE_OPERATOR, T_RCURLY, ch_str);
            break;
        case L'[': token = tok_new(TYPE_OPERATOR, T_LSQUARE, ch_str);
            break;
        case L']': token = tok_new(TYPE_OPERATOR, T_RSQUARE, ch_str);
            break;
        case L'"':
            tok_new(TYPE_DELIMITER, T_QUOTE, L"\"");
            token.literal = wcsdup(readstr(lexer));
            tok_new(TYPE_DELIMITER, T_QUOTE, L"\"");
            break;
        default:
            if(isalpha(lexer->ch)){
                readident(lexer);
            }
            else if(isdigit(lexer->ch)){
                readnum(lexer);
            }
            else{
                token = tok_new(TYPE_SERVICE, T_UNKNOWN, ch_str);
            }
            break;
    }
    return token;
}

void lex_free(Lexer *lex)
{
    if (lex) {
        free(lex->input);
        free(lex);
    }
}

void tok_free(Token *tok)
{
    if(tok) {
        free(tok->literal);
        free(tok);
    }
}

char* readident(Lexer *lexer)
{

}

char* readnum(Lexer *lexer)
{

}

wchar_t* readstr(Lexer *lexer) {
    size_t capacity = 32;
    size_t length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) return NULL;

    read_ch(lexer);

    while (lexer->ch != '"' && lexer->ch != '\0') {
        if (lexer->ch == '\\') {
            read_ch(lexer);
            switch (lexer->ch) {
                case 'n': buffer[length++] = '\n'; break;
                case 't': buffer[length++] = '\t'; break;
                case '"': buffer[length++] = '"'; break;
                case '\\': buffer[length++] = '\\'; break;
                default:
                    free(buffer);
                    return NULL;
            }
        } else {
            buffer[length++] = lexer->ch;
        }

        if (length >= capacity - 1) {
            capacity *= 2;
            char *new_buf = realloc(buffer, capacity);
            if (!new_buf) {
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }

        read_ch(lexer);
    }

    if (lexer->ch != '"') {
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';
    return buffer;
}
