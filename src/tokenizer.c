#include <stddef.h>
#include <wchar.h>
#include <stdlib.h>

#include "tokenizer.h"

#define DEBUG

#ifdef DEBUG

const char *token_type_to_str(enum category_tag category){
    static const char *names[] = {
        "SERVICE", "OPERATOR", "KEYWORD", "PAREN",
        "DELIMITER", "DATATYPE", "VALUE", "MODIFIER"};
    return (category < sizeof(names) / sizeof(names[0])) ? names[category] : "UNKNOWN";
}

#define print_token(t)                         \
wprintf(L"%s(%ls)\n", token_type_to_str((t)->category), (t)->literal)
#else
#define print_token(t)
#endif

const struct keyword operators[] = {
    {L"++", OP_INCREM, CATEGORY_OPERATOR}, {L"--", OP_DECREM,  CATEGORY_OPERATOR},
    {L"==", OP_EQ,     CATEGORY_OPERATOR}, {L"!=", OP_NEQ,     CATEGORY_OPERATOR},
    {L"+=", OP_ADD,    CATEGORY_OPERATOR}, {L"-=", OP_SUB,     CATEGORY_OPERATOR},
    {L"*=", OP_MUL,    CATEGORY_OPERATOR}, {L"/=", OP_DIV,     CATEGORY_OPERATOR},
    {L"&&", OP_AND,    CATEGORY_OPERATOR}, {L"||", OP_OR,      CATEGORY_OPERATOR},
    {L"<=", OP_LTE,    CATEGORY_OPERATOR}, {L">=", OP_GTE,     CATEGORY_OPERATOR},
    {L"..", OP_RANGE,  CATEGORY_OPERATOR}, {L"->", OP_RETTYPE, CATEGORY_OPERATOR},
    {L"%=", OP_MOD,    CATEGORY_OPERATOR},
};

const struct keyword keywords[] = {
    /* сontrol structures */
    {L"if",      KW_IF,      CATEGORY_KEYWORD}, {L"else",     KW_ELSE,      CATEGORY_KEYWORD},
    {L"elif",    KW_ELIF,    CATEGORY_KEYWORD}, {L"for",      KW_FOR,       CATEGORY_KEYWORD},
    {L"do",      KW_DO,      CATEGORY_KEYWORD}, {L"while",    KW_WHILE,     CATEGORY_KEYWORD},
    {L"func",    KW_FUNC,    CATEGORY_KEYWORD}, {L"return",   KW_RETURN,    CATEGORY_KEYWORD},
    {L"break",   KW_BREAK,   CATEGORY_KEYWORD}, {L"continue", KW_CONTINUE,  CATEGORY_KEYWORD},
    {L"default", KW_DEFAULT, CATEGORY_KEYWORD}, {L"match",    KW_MATCH,     CATEGORY_KEYWORD},
    {L"case",    KW_CASE,    CATEGORY_KEYWORD}, {L"struct",   KW_STRUCT,    CATEGORY_KEYWORD},
    {L"enum",    KW_ENUM,    CATEGORY_KEYWORD}, {L"union",    KW_UNION,     CATEGORY_KEYWORD},
    {L"import",  KW_IMPORT,  CATEGORY_KEYWORD}, {L"type",     KW_TYPE,      CATEGORY_KEYWORD},
    {L"trait",   KW_TRAIT,   CATEGORY_KEYWORD}, {L"try",      KW_TRY,       CATEGORY_KEYWORD},
    {L"catch",   KW_CATCH,   CATEGORY_KEYWORD}, {L"async",    KW_ASYNC,     CATEGORY_KEYWORD},
    {L"await",   KW_AWAIT,   CATEGORY_KEYWORD}, {L"test",     KW_TEST,      CATEGORY_KEYWORD},
    {L"assert",  KW_ASSERT,  CATEGORY_KEYWORD}, {L"verify",   KW_VERIFY,    CATEGORY_KEYWORD},
    {L"solve",   KW_SOLVE,   CATEGORY_KEYWORD}, {L"where",    KW_WHERE,     CATEGORY_KEYWORD},    
    {L"snapshot",KW_SNAPSHOT,CATEGORY_KEYWORD}, {L"rollback", KW_ROLLBACK,  CATEGORY_KEYWORD},
    {L"commit",  KW_COMMIT,  CATEGORY_KEYWORD}, {L"fork",     KW_FORK,      CATEGORY_KEYWORD},
    {L"merge",   KW_MERGE,   CATEGORY_KEYWORD}, {L"revert",   KW_REVERT,    CATEGORY_KEYWORD},
    {L"push",    KW_PUSH,    CATEGORY_KEYWORD}, {L"pull",     KW_PULL,      CATEGORY_KEYWORD},
    {L"clone",   KW_CLONE,   CATEGORY_KEYWORD}, {L"simulate", KW_SIMULATE,  CATEGORY_KEYWORD},
    {L"choose",  KW_CHOOSE,  CATEGORY_KEYWORD}, {L"scenarios",KW_SCENARIOS, CATEGORY_KEYWORD},  
    {L"branch",  KW_BRANCH,  CATEGORY_KEYWORD},  

    /* data types */
    {L"int",     DT_INT,    CATEGORY_DATATYPE}, {L"uint",    DT_UINT,    CATEGORY_DATATYPE},
    {L"float",   DT_FLOAT,  CATEGORY_DATATYPE}, {L"str",     DT_STR,     CATEGORY_DATATYPE},
    {L"bool",    DT_BOOL,   CATEGORY_DATATYPE}, {L"void",    DT_VOID,    CATEGORY_DATATYPE},
    {L"uni",     DT_UNI,    CATEGORY_DATATYPE}, {L"tensor",  DT_TENSOR,  CATEGORY_DATATYPE},
    {L"int8",    DT_INT8,   CATEGORY_DATATYPE}, {L"int16",   DT_INT16,   CATEGORY_DATATYPE},
    {L"int32",   DT_INT32,  CATEGORY_DATATYPE}, {L"int64",   DT_INT64,   CATEGORY_DATATYPE},
    {L"uint8",   DT_UINT8,  CATEGORY_DATATYPE}, {L"uint16",  DT_UINT16,  CATEGORY_DATATYPE},
    {L"uint32",  DT_UINT32, CATEGORY_DATATYPE}, {L"uint64",  DT_UINT64,  CATEGORY_DATATYPE},
    {L"float32", DT_FLOAT32,CATEGORY_DATATYPE}, {L"float64", DT_FLOAT64, CATEGORY_DATATYPE},
    
    /* modifiers */
    {L"var",   MOD_VAR,   CATEGORY_MODIFIER}, {L"const",  MOD_CONST,  CATEGORY_MODIFIER},
    {L"final", MOD_FINAL, CATEGORY_MODIFIER}, {L"static", MOD_STATIC, CATEGORY_MODIFIER},
    {L"event", MOD_EVENT, CATEGORY_MODIFIER}, {L"signal", MOD_SIGNAL, CATEGORY_MODIFIER},
};

const size_t operators_count = sizeof(operators) / sizeof(struct keyword);
const size_t keywords_count  = sizeof(keywords)  / sizeof(struct keyword);

struct token new_token(const enum category_tag category, const int type, const wchar_t *literal)
{
    struct token tok = {
        .category = category,
        .literal = NULL};

    if (literal){
        tok.literal = wcsdup(literal);
        if (!tok.literal){
            tok.category = CATEGORY_SERVICE;
            tok.service = SERV_ILLEGAL;
            return tok;
        }
    }

    switch (category){
        case CATEGORY_SERVICE:    tok.service =    (enum category_tag)type; break;
        case CATEGORY_OPERATOR:   tok.oper =       (enum operator_category)type; break;
        case CATEGORY_KEYWORD:    tok.keyword =    (enum keyword_category)type; break;
        case CATEGORY_PAREN:      tok.paren =      (enum paren_category)type; break;
        case CATEGORY_DELIMITER:  tok.delim =      (enum delimiter_category)type; break;
        case CATEGORY_DATATYPE:   tok.datatype =   (enum datatype_category)type; break;
        case CATEGORY_VALUE:      tok.value =      (enum value_category)type; break;
        case CATEGORY_MODIFIER:   tok.modifier =   (enum modifier_category)type; break;
    }

#ifdef DEBUG
    print_token(&tok);
#endif

    return tok;
}

void free_token(struct token *tok)
{
    if (tok){
        free(tok->literal);
        tok->literal = NULL;
    }
}