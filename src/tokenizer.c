#include <stddef.h>
#include <wchar.h>
#include <stdlib.h>

#include "tokenizer.h"

#define DEBUG

#ifdef DEBUG

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

const Keyword operators[] = {
    {L"++", T_INCREM},  {L"--", T_DECREM},
    {L"==", T_EQ},      {L"!=", T_NEQ},
    {L"+=", T_ADD},     {L"-=", T_SUB},
    {L"*=", T_MUL},     {L"/=", T_DIV},
    {L"&&", T_AND},     {L"||", T_OR},
    {L"<=", T_LTE},     {L">=", T_GTE},
    {L"..", T_RANGE},   {L"->", T_RETTYPE},
    {L"%=", T_MOD},
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
    {L"solve", T_SOLVE},    {L"where", T_WHERE},    
    {L"snapshot", T_SNAPSHOT},{L"rollback", T_ROLLBACK},
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
    {L"int8", T_INT8},      {L"int16", T_INT16},
    {L"int32", T_INT32},    {L"int64", T_INT64},
    {L"uint8", T_UINT8},    {L"uint16", T_UINT16},
    {L"uint32", T_UINT32},  {L"uint64", T_UINT64},
    {L"float32", T_FLOAT32},{L"float64", T_FLOAT64},
};

const Keyword modifiers[] = {
    {L"var", T_VAR},        {L"const", T_CONST},
    {L"final", T_FINAL},    {L"static", T_STATIC},
    {L"event", T_EVENT},    {L"signal", T_SIGNAL},
};

const size_t operators_count = sizeof(operators) / sizeof(Keyword);
const size_t keywords_count  = sizeof(keywords)  / sizeof(Keyword);
const size_t datatypes_count = sizeof(datatypes)/ sizeof(Keyword);
const size_t modifiers_count = sizeof(modifiers) / sizeof(Keyword);

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

void tok_free(Token *tok)
{
    if (tok){
        free(tok->literal);
        tok->literal = NULL;
    }
}