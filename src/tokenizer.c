#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tokenizer.h"
#define DEBUG
#include "debug.h"


const struct keyword operators[] = {
    {"++", OP_INCREM, CATEGORY_OPERATOR}, {"--", OP_DECREM,  CATEGORY_OPERATOR},
    {"==", OP_EQ,     CATEGORY_OPERATOR}, {"!=", OP_NEQ,     CATEGORY_OPERATOR},
    {"+=", OP_ADD,    CATEGORY_OPERATOR}, {"-=", OP_SUB,     CATEGORY_OPERATOR},
    {"*=", OP_MUL,    CATEGORY_OPERATOR}, {"/=", OP_DIV,     CATEGORY_OPERATOR},
    {"&&", OP_AND,    CATEGORY_OPERATOR}, {"||", OP_OR,      CATEGORY_OPERATOR},
    {"<=", OP_LTE,    CATEGORY_OPERATOR}, {">=", OP_GTE,     CATEGORY_OPERATOR},
    {"..", OP_RANGE,  CATEGORY_OPERATOR}, {"->", OP_RETTYPE, CATEGORY_OPERATOR},
    {"%=", OP_MOD,    CATEGORY_OPERATOR},
};

const struct keyword keywords[] = {
    /* сontrol structures */
    {"if",      KW_IF,      CATEGORY_KEYWORD}, {"else",     KW_ELSE,      CATEGORY_KEYWORD},
    {"elif",    KW_ELIF,    CATEGORY_KEYWORD}, {"for",      KW_FOR,       CATEGORY_KEYWORD},
    {"do",      KW_DO,      CATEGORY_KEYWORD}, {"while",    KW_WHILE,     CATEGORY_KEYWORD},
    {"func",    KW_FUNC,    CATEGORY_KEYWORD}, {"return",   KW_RETURN,    CATEGORY_KEYWORD},
    {"break",   KW_BREAK,   CATEGORY_KEYWORD}, {"continue", KW_CONTINUE,  CATEGORY_KEYWORD},
    {"default", KW_DEFAULT, CATEGORY_KEYWORD}, {"match",    KW_MATCH,     CATEGORY_KEYWORD},
    {"case",    KW_CASE,    CATEGORY_KEYWORD}, {"struct",   KW_STRUCT,    CATEGORY_KEYWORD},
    {"enum",    KW_ENUM,    CATEGORY_KEYWORD}, {"union",    KW_UNION,     CATEGORY_KEYWORD},
    {"import",  KW_IMPORT,  CATEGORY_KEYWORD}, {"type",     KW_TYPE,      CATEGORY_KEYWORD},
    {"trait",   KW_TRAIT,   CATEGORY_KEYWORD}, {"try",      KW_TRY,       CATEGORY_KEYWORD},
    {"catch",   KW_CATCH,   CATEGORY_KEYWORD}, {"async",    KW_ASYNC,     CATEGORY_KEYWORD},
    {"await",   KW_AWAIT,   CATEGORY_KEYWORD}, {"test",     KW_TEST,      CATEGORY_KEYWORD},
    {"assert",  KW_ASSERT,  CATEGORY_KEYWORD}, {"verify",   KW_VERIFY,    CATEGORY_KEYWORD},
    {"solve",   KW_SOLVE,   CATEGORY_KEYWORD}, {"where",    KW_WHERE,     CATEGORY_KEYWORD},    
    {"snapshot",KW_SNAPSHOT,CATEGORY_KEYWORD}, {"rollback", KW_ROLLBACK,  CATEGORY_KEYWORD},
    {"commit",  KW_COMMIT,  CATEGORY_KEYWORD}, {"fork",     KW_FORK,      CATEGORY_KEYWORD},
    {"merge",   KW_MERGE,   CATEGORY_KEYWORD}, {"revert",   KW_REVERT,    CATEGORY_KEYWORD},
    {"push",    KW_PUSH,    CATEGORY_KEYWORD}, {"pull",     KW_PULL,      CATEGORY_KEYWORD},
    {"clone",   KW_CLONE,   CATEGORY_KEYWORD}, {"simulate", KW_SIMULATE,  CATEGORY_KEYWORD},
    {"choose",  KW_CHOOSE,  CATEGORY_KEYWORD}, {"scenarios",KW_SCENARIOS, CATEGORY_KEYWORD},  
    {"branch",  KW_BRANCH,  CATEGORY_KEYWORD},  

    /* data types */
    {"int",     DT_INT,    CATEGORY_DATATYPE}, {"uint",    DT_UINT,    CATEGORY_DATATYPE},
    {"float",   DT_FLOAT,  CATEGORY_DATATYPE}, {"str",     DT_STR,     CATEGORY_DATATYPE},
    {"bool",    DT_BOOL,   CATEGORY_DATATYPE}, {"void",    DT_VOID,    CATEGORY_DATATYPE},
    {"uni",     DT_UNI,    CATEGORY_DATATYPE}, {"tensor",  DT_TENSOR,  CATEGORY_DATATYPE},
    {"int8",    DT_INT8,   CATEGORY_DATATYPE}, {"int16",   DT_INT16,   CATEGORY_DATATYPE},
    {"int32",   DT_INT32,  CATEGORY_DATATYPE}, {"int64",   DT_INT64,   CATEGORY_DATATYPE},
    {"uint8",   DT_UINT8,  CATEGORY_DATATYPE}, {"uint16",  DT_UINT16,  CATEGORY_DATATYPE},
    {"uint32",  DT_UINT32, CATEGORY_DATATYPE}, {"uint64",  DT_UINT64,  CATEGORY_DATATYPE},
    {"float32", DT_FLOAT32,CATEGORY_DATATYPE}, {"float64", DT_FLOAT64, CATEGORY_DATATYPE},
    
    /* modifiers */
    {"var",   MOD_VAR,   CATEGORY_MODIFIER}, {"const",  MOD_CONST,  CATEGORY_MODIFIER},
    {"final", MOD_FINAL, CATEGORY_MODIFIER}, {"static", MOD_STATIC, CATEGORY_MODIFIER},
    {"event", MOD_EVENT, CATEGORY_MODIFIER}, {"signal", MOD_SIGNAL, CATEGORY_MODIFIER},
};

const size_t operators_count = sizeof(operators) / sizeof(struct keyword);
const size_t keywords_count  = sizeof(keywords)  / sizeof(struct keyword);

struct token new_token(const enum category_tag category, const int type, const char *literal)
{
    struct token tok = {
        .category = category,
        .literal = NULL};

    if (literal){
        tok.literal = strdup(literal);
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