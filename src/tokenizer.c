#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tokenizer.h"
#include "utils.h"
#include "debug.h"

#define C_OP (CATEGORY_OPERATOR)
#define C_KW (CATEGORY_KEYWORD)
#define C_DT (CATEGORY_DATATYPE)
#define C_MD (CATEGORY_MODIFIER)
#define C_SV (CATEGORY_SERVICE)
#define C_LT (CATEGORY_LITERAL)
#define C_PR (CATEGORY_PAREN)
#define C_DL (CATEGORY_DELIMITER)

const struct keyword operators[] = {
    {"++", OPER_INCREM, C_OP}, {"--", OPER_DECREM,  C_OP},
    {"==", OPER_EQ,     C_OP}, {"!=", OPER_NEQ,     C_OP},
    {"+=", OPER_ADD,    C_OP}, {"-=", OPER_SUB,     C_OP},
    {"*=", OPER_MUL,    C_OP}, {"/=", OPER_DIV,     C_OP},
    {"%=", OPER_MOD,    C_OP}, {"&&", OPER_AND,     C_OP},
    {"||", OPER_OR,     C_OP}, {"<=", OPER_LTE,     C_OP},
    {">=", OPER_GTE,    C_OP}, {"..", OPER_RANGE,   C_OP},
    {"=>", OPER_ARROW,  C_OP}
};

const struct keyword keywords[] = {
    /* сontrol structures */
    {"if",      KW_IF,      C_KW}, {"else",     KW_ELSE,      C_KW},
    {"elif",    KW_ELIF,    C_KW}, {"for",      KW_FOR,       C_KW},
    {"do",      KW_DO,      C_KW}, {"while",    KW_WHILE,     C_KW},
    {"func",    KW_FUNC,    C_KW}, {"return",   KW_RETURN,    C_KW},
    {"break",   KW_BREAK,   C_KW}, {"continue", KW_CONTINUE,  C_KW},
    {"default", KW_DEFAULT, C_KW}, {"match",    KW_MATCH,     C_KW},
    {"case",    KW_CASE,    C_KW}, {"struct",   KW_STRUCT,    C_KW},
    {"enum",    KW_ENUM,    C_KW}, {"union",    KW_UNION,     C_KW},
    {"import",  KW_IMPORT,  C_KW}, {"type",     KW_TYPE,      C_KW},
    {"trait",   KW_TRAIT,   C_KW}, {"try",      KW_TRY,       C_KW},
    {"catch",   KW_CATCH,   C_KW}, {"async",    KW_ASYNC,     C_KW},
    {"await",   KW_AWAIT,   C_KW}, {"test",     KW_TEST,      C_KW},
    {"assert",  KW_ASSERT,  C_KW}, {"verify",   KW_VERIFY,    C_KW},
    {"solve",   KW_SOLVE,   C_KW}, {"where",    KW_WHERE,     C_KW},    
    {"snapshot",KW_SNAPSHOT,C_KW}, {"rollback", KW_ROLLBACK,  C_KW},
    {"commit",  KW_COMMIT,  C_KW}, {"fork",     KW_FORK,      C_KW},
    {"merge",   KW_MERGE,   C_KW}, {"revert",   KW_REVERT,    C_KW},
    {"push",    KW_PUSH,    C_KW}, {"pull",     KW_PULL,      C_KW},
    {"clone",   KW_CLONE,   C_KW}, {"simulate", KW_SIMULATE,  C_KW},
    {"choose",  KW_CHOOSE,  C_KW}, {"scenarios",KW_SCENARIOS, C_KW},  
    {"branch",  KW_BRANCH,  C_KW},  

    /* data types */
    {"int",     DT_INT,    C_DT}, {"uint",    DT_UINT,    C_DT},
    {"float",   DT_FLOAT,  C_DT}, {"str",     DT_STR,     C_DT},
    {"bool",    DT_BOOL,   C_DT}, {"void",    DT_VOID,    C_DT},
    // {"int8",    DT_INT8,   C_DT}, {"int16",   DT_INT16,   C_DT},
    // {"int32",   DT_INT32,  C_DT}, {"int64",   DT_INT64,   C_DT},
    // {"uint8",   DT_UINT8,  C_DT}, {"uint16",  DT_UINT16,  C_DT},
    // {"uint32",  DT_UINT32, C_DT}, {"uint64",  DT_UINT64,  C_DT},
    // {"float32", DT_FLOAT32,C_DT}, {"float64", DT_FLOAT64, C_DT},
    
    /* modifiers */
    {"var",   MOD_VAR,   C_MD}, {"const",  MOD_CONST,  C_MD},
    {"final", MOD_FINAL, C_MD}, {"static", MOD_STATIC, C_MD},
};

const size_t operators_count = sizeof(operators) / sizeof(struct keyword);
const size_t keywords_count  = sizeof(keywords)  / sizeof(struct keyword);

struct token new_token(const enum category_tag category, const int type, const char *literal)
{
    struct token tok = {.category = category, .literal = NULL};

    if (literal){
        tok.literal = strdup(literal);
        if (!tok.literal){
            tok.category = C_SV;
            tok.type_service = SERV_ILLEGAL;
            return tok;
        }
    }

    switch (category){
        case C_SV: tok.type_service =  (enum category_service)type; break;
        case C_OP: tok.type_operator = (enum category_operator)type; break;
        case C_KW: tok.type_keyword =  (enum category_keyword)type; break;
        case C_PR: tok.type_paren =    (enum category_paren)type; break;
        case C_DL: tok.type_delim =    (enum category_delimiter)type; break;
        case C_DT: tok.type_datatype = (enum category_datatype)type; break;
        case C_LT: tok.type_literal =  (enum category_literal)type; break;
        case C_MD: tok.type_modifier = (enum category_modifier)type; break;
    }

#ifdef DEBUG
    print_token(&tok);
#endif

    return tok;
}

void free_token(struct token *tok)
{
    if(!tok) return;
    free(tok->literal);
    tok->literal = NULL;
}
