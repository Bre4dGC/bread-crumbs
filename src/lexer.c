#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/lexer.h"

Lexer lexer = {.input=NULL, .pos=0, .nextpos=1, .ch=0};

const Operator operators[] = {
    {T_LTE,   "<="},
    {T_GTE,   ">="},
    {T_EQ,    "=="},
    {T_NEQ,   "!="},
    {T_ADD,   "+="},
	{T_SUB,   "-="},
	{T_MUL,   "*="},
	{T_DIV,   "/="},
	{T_AND,   "&&"},
	{T_OR,    "||"},
	{T_ELLIP, ".."},
};

const Keyword keywords[] = {
    {T_IF,       "if"},
    {T_ELSE,     "else"},
    {T_ELIF,     "elif"},
    {T_FOR,      "for"},
    {T_DO,       "do"},
    {T_WHILE,    "while"},
    {T_FUNC,     "func"},
    {T_RETURN,   "return"},
    {T_BREAK,    "break"},
    {T_CONTINUE, "continue"},
    {T_DEFAULT,  "default"},
    {T_MATCH,    "match"},
    {T_CASE,     "case"},
    {T_STRUCT,   "struct"},
    {T_ENUM,     "enum"},
    {T_UNION,    "union"},
    {T_IMPORT,   "import"},
    {T_TYPE,     "type"},
    {T_TRAIT,    "trait"},
    {T_TRY,      "try"},
    {T_CATCH,    "catch"},
};

const size_t operators_count = sizeof(operators) / sizeof(Operator);
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

void lex_free(Lexer *lex)
{
    if (lex) {
        free(lex->input);
        free(lex);
    }
}

Token tok_new(T_TypeTag ttag, wchar_t *liter)
{

}

void tok_free(Token *tok)
{
    if(tok) {
        free(tok->literal);
        free(tok);
    }
}

char* lex_readident(Lexer *lex)
{

}

char* lex_readnum(Lexer *lex)
{

}

char* lex_readstr(Lexer *lex)
{

}

Token lex_nexttok(Lexer *lexer)
{

}
