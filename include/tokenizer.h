#pragma once

enum service_category{
	SERV_ILLEGAL, SERV_IDENT, SERV_COMMENT, SERV_EOF
};

enum operator_category{
    OP_PLUS,    OP_MINUS,
    OP_ASTERISK,  OP_SLASH,
    OP_DOT,     OP_COMMA,
    OP_LANGLE,  OP_RANGLE,
    OP_ASSIGN,  OP_COLON,
    OP_NOT,     OP_QUESTION,
    OP_RANGE,   OP_RETTYPE,
    OP_ADD,     OP_SUB,
    OP_MUL,     OP_DIV,
    OP_PERCENT, OP_MOD,
    OP_AND,     OP_OR,
    OP_EQ,      OP_NEQ,
    OP_LTE,     OP_GTE,
    OP_INCREM,  OP_DECREM,
};

enum keyword_category {
    KW_IF,       KW_ELIF,     KW_ELSE,
    KW_FOR,      KW_DO,       KW_WHILE,
    KW_FUNC,     KW_RETURN,   KW_BREAK,
    KW_CONTINUE, KW_DEFAULT,  KW_MATCH,
    KW_CASE,     KW_STRUCT,   KW_ENUM,
    KW_UNION,    KW_IMPL,     KW_IMPORT,
    KW_TYPE,     KW_TRAIT,    KW_TRY,
    KW_CATCH,    KW_ASYNC,    KW_AWAIT,
    KW_TEST,     KW_ASSERT,   KW_VERIFY,
    KW_WHERE,    KW_SOLVE,    KW_SNAPSHOT,
    KW_ROLLBACK, KW_COMMIT,   KW_FORK,
    KW_BRANCH,   KW_MERGE,    KW_REVERT,
    KW_PUSH,     KW_PULL,     KW_CLONE,
    KW_SIMULATE, KW_SCENARIOS,KW_CHOOSE,
};

enum paren_category {
    PAR_LPAREN,   PAR_RPAREN,   // ()
    PAR_LBRACE,   PAR_RBRACE,   // {}
    PAR_LBRACKET, PAR_RBRACKET  // []
};

enum delimiter_category {
    DELIM_QUOTE, DELIM_SQUOTE
};

enum datatype_category {
    /* basic types */
    DT_INT,  DT_UINT, DT_FLOAT,
    DT_VOID, DT_BOOL, DT_STR, 
    DT_UNI,  DT_TENSOR, 

    /* exact types */
    DT_INT8,    DT_INT16,  DT_INT32,  DT_INT64,
    DT_UINT8,   DT_UINT16, DT_UINT32, DT_UINT64,
    DT_FLOAT32, DT_FLOAT64,
};

enum value_category {
    VAL_NUMBER, VAL_DECIMAL,
    VAL_CHAR,   VAL_STRING,
    VAL_TRUE,   VAL_FALSE,
    VAL_HEX,    VAL_BIN,
    VAL_NULL,
};

enum modifier_category {
    MOD_VAR,    MOD_CONST, MOD_FINAL, 
    MOD_STATIC, MOD_EVENT, MOD_SIGNAL
};

enum category_tag {
    CATEGORY_SERVICE,   CATEGORY_OPERATOR,
    CATEGORY_KEYWORD,   CATEGORY_PAREN,
    CATEGORY_DELIMITER, CATEGORY_DATATYPE,
    CATEGORY_VALUE,     CATEGORY_MODIFIER,
};

struct keyword {
    wchar_t *literal;
    enum category_tag category;
    int type;
};

struct token {
    enum category_tag category;
	union {
        enum service_category service;
        enum operator_category oper;
        enum keyword_category keyword;
        enum paren_category paren;
        enum delimiter_category delim;
        enum datatype_category datatype;
        enum value_category value;
        enum modifier_category modifier;
	};
	wchar_t *literal;
};

extern const struct keyword operators[];
extern const struct keyword keywords[];

extern const size_t operators_count;
extern const size_t keywords_count;

struct token new_token(const enum category_tag category, const int type, const wchar_t* literal);
void free_token(struct token* token);