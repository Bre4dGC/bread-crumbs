#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/lexer.h"

Token token;
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

Lexer* lex_new(const char *input)
{
    if(!lexer.input) free(lexer.input);
    lexer.input = (char *)malloc(sizeof(input)+1);
    strcpy(lexer.input, input);
    lexer.pos = lexer.nextpos;
    lexer.nextpos += 1;
    lexer.ch = input[lexer.pos];
    return &lexer;
}

char* lex_read_id(Lexer *lexer)
{
    return NULL;
}

char* lex_read_num(Lexer *lexer)
{
    return NULL;
}

char* lex_read_str(Lexer *lexer, const TDelimiterType quote_type)
{
    return NULL;
}

int lex_peek_ch(const Lexer *lexer, char exp_ch)
{
    if(lexer->input[lexer->nextpos] == exp_ch)
        return 0;
    else
        return -1;
}

void lex_skip_space(Lexer *lexer)
{
    while(isspace(lexer->ch)) lexer->pos++;
    if(lexer->ch == '#') {
        // lexer = lex_new(); return;
    }
}

Token tok_new(const TokenTypeTag tag, int type)
{

}

Token lex_next_tok(Lexer *lexer)
{
    lex_skip_space(lexer);

    // if (lexer->input[lexer->pos] == '\0') {
    //     token.tag = TOK_SERVICE;
    //     return toke;
    // }

    switch (lexer->ch) {
        case '+':
            if(lex_peek_ch(lexer, '=')){
                token = tok_new(TOK_OPERATOR, T_ADD);
            }
            else {
                token = tok_new(TOK_OPERATOR, T_PLUS);
            }
            break;
    }
}

Token lex_oper_tok()

void lex_free(Lexer *lexer)
{
    free(lexer->input);
    free(lexer);
}
