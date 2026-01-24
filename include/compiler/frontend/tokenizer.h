#pragma once
#include <stddef.h>

enum category_service {
	SERV_ILLEGAL, SERV_COMMENT, SERV_EOF
};

enum category_operator {
    // basic arithmetic operators
    OPER_PLUS,      // +
    OPER_MINUS,     // -
    OPER_ASTERISK,  // *
    OPER_SLASH,     // /
    OPER_PERCENT,   // %

    // assignment operators
    OPER_ASSIGN,    // =
    OPER_ADD,       // +=
    OPER_SUB,       // -=
    OPER_MUL,       // *=
    OPER_DIV,       // /=
    OPER_MOD,       // %=

    // comparison operators
    OPER_EQ,        // ==
    OPER_NEQ,       // !=
    OPER_LANGLE,    // <
    OPER_RANGLE,    // >
    OPER_LTE,       // <=
    OPER_GTE,       // >=

    // logical operators
    OPER_AND,       // &&
    OPER_OR,        // ||
    OPER_NOT,       // !

    // increment/decrement
    OPER_INCREM,    // ++
    OPER_DECREM,    // --

    // punctuation
    OPER_DOT,       // .
    OPER_COMMA,     // ,
    OPER_COLON,     // :
    OPER_SEMICOLON, // ;
    OPER_QUESTION,  // ?
    OPER_RANGE,     // ..
    OPER_ARROW,     // ->
};

enum category_keyword {
    // base
    KW_IF,       KW_ELIF,     KW_ELSE,
    KW_FOR,      KW_DO,       KW_WHILE,
    KW_FUNC,     KW_STRUCT,   KW_ENUM,
    KW_MATCH,    KW_CASE,     KW_DEFAULT,
    KW_RETURN,   KW_BREAK,    KW_CONTINUE,

    // advanced
    KW_TRAIT,    KW_IMPL,     KW_SELF,
    KW_IMPORT,   KW_MODULE,   KW_TYPE,
    KW_TRY,      KW_CATCH,    KW_FINALLY,
    KW_ASYNC,    KW_AWAIT,    KW_YIELD,
    KW_TEST,     KW_ASSERT,   KW_VERIFY,
    KW_WHERE,    KW_SOLVE,    KW_SNAPSHOT,
    KW_ROLLBACK, KW_COMMIT,   KW_FORK,
    KW_BRANCH,   KW_MERGE,    KW_REVERT,
    KW_PUSH,     KW_PULL,     KW_CLONE,
    KW_SIMULATE, KW_SCENARIOS,KW_CHOOSE,

    // special
    KW_NAMEOF,   KW_TYPEOF,
};

enum category_paren {
    PAR_LPAREN,   PAR_RPAREN,   // ()
    PAR_LBRACE,   PAR_RBRACE,   // {}
    PAR_LBRACKET, PAR_RBRACKET  // []
};

enum category_delimiter {
    DELIM_DQUOTE, DELIM_SQUOTE
};

enum category_datatype {
    DT_INT,   DT_UINT,
    DT_SHORT, DT_USHORT,
    DT_LONG,  DT_ULONG,
    DT_CHAR,  DT_BYTE,
    DT_FLOAT, DT_DECIMAL,
    DT_STR,   DT_BOOL,
    DT_VOID,  DT_ANY,
};

enum category_literal {
    LIT_NULL,
    LIT_IDENT,  LIT_NUMBER,
    LIT_CHAR,   LIT_STRING,
    LIT_TRUE,   LIT_FALSE,
    LIT_FLOAT,  LIT_INFINITY,
    LIT_HEX,    LIT_BIN,     LIT_OCT,
};

enum category_modifier {
    MOD_VAR, MOD_CONST, MOD_FINAL, MOD_STATIC,
};

enum category_tag {
    CAT_SERVICE,   CAT_OPERATOR,
    CAT_KEYWORD,   CAT_PAREN,
    CAT_DELIMITER, CAT_DATATYPE,
    CAT_LITERAL,   CAT_MODIFIER,
};

typedef struct {
	const char* literal;
    int type;
    enum category_tag category;
} token_t;

void init_tokens(void);
token_t new_token(const enum category_tag category, const int type, const char* literal);
token_t* find_token(const char* potential);
void free_tokens(void);
