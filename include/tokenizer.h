#pragma once

enum category_service{
	SERV_ILLEGAL, SERV_COMMENT, SERV_EOF
};

enum category_operator{
    OPER_PLUS,   OPER_MINUS,
    OPER_SLASH,  OPER_ASTERISK,
    OPER_DOT,    OPER_COMMA,
    OPER_LANGLE, OPER_RANGLE,
    OPER_ASSIGN,  
    OPER_COLON,  OPER_SEMICOLON,
    OPER_NOT,    OPER_QUESTION,
    OPER_ADD,    OPER_SUB,
    OPER_MUL,    OPER_DIV,
    OPER_MOD,    OPER_PERCENT,
    OPER_AND,    OPER_OR,
    OPER_EQ,     OPER_NEQ,
    OPER_LTE,    OPER_GTE,
    OPER_INCREM, OPER_DECREM,
    OPER_RANGE,  OPER_ARROW,
};

enum category_keyword {
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

enum category_paren {
    PAR_LPAREN,   PAR_RPAREN,   // ()
    PAR_LBRACE,   PAR_RBRACE,   // {}
    PAR_LBRACKET, PAR_RBRACKET  // []
};

enum category_delimiter {
    DELIM_QUOTE, DELIM_SQUOTE
};

enum category_datatype {
    /* basic types */
    DT_VOID, DT_BOOL, DT_STR,
    DT_INT,  DT_UINT, DT_FLOAT,

    /* exact types */
    DT_INT8,    DT_INT16,  DT_INT32,  DT_INT64,
    DT_UINT8,   DT_UINT16, DT_UINT32, DT_UINT64,
    DT_FLOAT32, DT_FLOAT64,
};

enum category_literal {
    LIT_IDENT,  LIT_NUMBER,
    LIT_CHAR,   LIT_STRING,
    LIT_FLOAT,  LIT_HEX,    LIT_BIN,
    LIT_TRUE,   LIT_FALSE,  LIT_NULL,
};

enum category_modifier {
    MOD_VAR, MOD_CONST, MOD_FINAL, MOD_STATIC,
};

enum category_tag {
    CATEGORY_SERVICE,   CATEGORY_OPERATOR,
    CATEGORY_KEYWORD,   CATEGORY_PAREN,
    CATEGORY_DELIMITER, CATEGORY_DATATYPE,
    CATEGORY_LITERAL,   CATEGORY_MODIFIER,
};

struct keyword {
    char* literal;
    int type;
    enum category_tag category;
};

struct token {
    enum category_tag category;
	union {
        enum category_service type_service;
        enum category_operator type_operator;
        enum category_keyword type_keyword;
        enum category_paren type_paren;
        enum category_delimiter type_delim;
        enum category_datatype type_datatype;
        enum category_literal type_literal;
        enum category_modifier type_modifier;
	};
	char* literal;
};

extern const struct keyword operators[];
extern const struct keyword keywords[];

extern const size_t operators_count;
extern const size_t keywords_count;

struct token new_token(const enum category_tag category, const int type, const char* literal);
void free_token(struct token* token);
