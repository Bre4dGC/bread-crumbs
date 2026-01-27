#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "compiler/core/hash_table.h"
#include "compiler/frontend/tokenizer.h"
#ifdef DEBUG
#include "common/debug.h"
#endif

#define C_OP (CAT_OPERATOR)
#define C_KW (CAT_KEYWORD)
#define C_DT (CAT_DATATYPE)
#define C_MD (CAT_MODIFIER)
#define C_LT (CAT_LITERAL)
#define C_PR (CAT_PAREN)
#define C_DL (CAT_DELIMITER)

hash_table_t* tokens_table = NULL;

void init_tokens(void)
{
    if(tokens_table != NULL) return;

    static token_t tokens[] = {
        /* operators */
        {"++", OPER_INCREM, C_OP}, {"--", OPER_DECREM, C_OP},
        {"==", OPER_EQ,     C_OP}, {"!=", OPER_NEQ,    C_OP},
        {"+=", OPER_ADD,    C_OP}, {"-=", OPER_SUB,    C_OP},
        {"*=", OPER_MUL,    C_OP}, {"/=", OPER_DIV,    C_OP},
        {"%=", OPER_MOD,    C_OP}, {"&&", OPER_AND,    C_OP},
        {"||", OPER_OR,     C_OP}, {"<=", OPER_LTE,    C_OP},
        {">=", OPER_GTE,    C_OP}, {"..", OPER_RANGE,  C_OP},
        {"->", OPER_ARROW,  C_OP},

        /* сontrol structures */
        {"if",      KW_IF,      C_KW}, {"else",     KW_ELSE,      C_KW},
        {"elif",    KW_ELIF,    C_KW}, {"for",      KW_FOR,       C_KW},
        {"do",      KW_DO,      C_KW}, {"while",    KW_WHILE,     C_KW},
        {"func",    KW_FUNC,    C_KW}, {"return",   KW_RETURN,    C_KW},
        {"break",   KW_BREAK,   C_KW}, {"continue", KW_CONTINUE,  C_KW},
        {"default", KW_DEFAULT, C_KW},
        {"match",   KW_MATCH,   C_KW}, {"case",     KW_CASE,      C_KW},
        {"struct",  KW_STRUCT,  C_KW}, {"enum",     KW_ENUM,      C_KW},
        {"import",  KW_IMPORT,  C_KW}, {"module",   KW_MODULE,    C_KW},
        {"type",    KW_TYPE,    C_KW},
        {"trait",   KW_TRAIT,   C_KW}, {"impl",     KW_IMPL,      C_KW},
        {"try",     KW_TRY,     C_KW}, {"catch",    KW_CATCH,     C_KW},
        {"finally", KW_FINALLY, C_KW},
        {"async",   KW_ASYNC,   C_KW}, {"await",    KW_AWAIT,     C_KW},
        {"yield",   KW_YIELD,   C_KW},

        /* data types */
        {"int",     DT_INT,     C_DT}, {"uint",    DT_UINT,       C_DT},
        {"short",   DT_SHORT,   C_DT}, {"ushort",  DT_USHORT,     C_DT},
        {"long",    DT_LONG,    C_DT}, {"ulong",   DT_ULONG,      C_DT},
        {"char",    DT_CHAR,    C_DT}, {"byte",    DT_BYTE,       C_DT},
        {"float",   DT_FLOAT,   C_DT}, {"decimal", DT_DECIMAL,    C_DT},
        {"str",     DT_STR,     C_DT}, {"bool",    DT_BOOL,       C_DT},
        {"void",     DT_VOID,   C_DT}, {"any",     DT_ANY,        C_DT},

        /* modifiers */
        {"var",     MOD_VAR,    C_MD}, {"const",   MOD_CONST,     C_MD},
        {"final",   MOD_FINAL,  C_MD}, {"static",  MOD_STATIC,    C_MD},

        /* literals */
        {"true",    LIT_TRUE,   C_LT}, {"false",    LIT_FALSE,    C_LT},
        {"null",    LIT_NULL,   C_LT}, {"infinity", LIT_INFINITY, C_LT},

        /* specials */
        {"nameof",  KW_NAMEOF,  C_KW}, {"typeof",   KW_TYPEOF,    C_KW},
    };

    const size_t tokens_count = sizeof(tokens) / sizeof(tokens[0]);

    tokens_table = new_hashtable();

    for(size_t i = 0; i < tokens_count; ++i){
        ht_insert(tokens_table, tokens[i].literal, &tokens[i]);
    }
}

token_t new_token(const enum category_tag category, const int type, const char *literal)
{
    token_t token = {
        .literal = literal,
        .type = type,
        .category = category,
    };

#ifdef DEBUG
    print_token(token);
#endif

    return token;
}

token_t* find_token(const char* literal)
{
    return (token_t*)ht_lookup(tokens_table, literal);
}

void free_tokens(void)
{
    free_hashtable(tokens_table);
}
