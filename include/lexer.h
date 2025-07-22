#pragma once

typedef enum {
	T_ILLEGAL,  // unknown
	T_IDENT,    // identificator
	T_EOF       // end of file
} TServiceType;

typedef enum {
    T_MUL,      // *
	T_DIV,      // /
	T_ADD,      // +
	T_SUB,      // -
	T_ASSIGN,   // =
	T_EQ,       // ==
	T_NEQ,      // !=
	T_LT,       // <
	T_HT,       // >
	T_LTEQ,     // <=
	T_HTEQ,     // >=
	T_POINT,    // .
	T_COMMA,    // ,
	T_COLON,    // :
} TOperatorType;

typedef enum {
    T_IF,       // if
	T_ELIF,     // elif
	T_ELSE,     // else
	T_WHILE,    // while
	T_FOR,      // for
	T_FUNC,     // fn
	T_STRUCT    // struct
} TKeywordType;

typedef enum {
    T_LROUND,   // (
    T_RROUND,   // )
    T_LCURLY,   // {
    T_RCURLY,   // }
    T_LSQUARE,  // [
    T_RSQUARE,  // ]
} TParenType;

typedef enum {
    /* basic types */
    T_INT, T_UINT, T_FLOAT,
	T_VOID, T_BOOL, T_STR,

    /* exact types */
    T_INT8,    T_INT16,  T_INT32,  T_INT64,
    T_UINT8,   T_UINT16, T_UINT32, T_UINT64,
    T_FLOAT32, T_FLOAT64,
} TDataType;

typedef enum {
    T_LIST, T_STACK, T_MAP, T_VECTOR, T_TUPLE, T_ARRAY, T_SET
} TCollectionType;

typedef enum {
    T_VAR, T_CONST, T_FINAL, T_STATIC, T_PUBLIC, T_PRIVATE
} TModifierType;

typedef enum {
    TYPE_SERVICE, TYPE_OPERATOR, TYPE_KEYWORD, TYPE_PAREN, TYPE_DATATYPE, TYPE_COLLECTION, TYPE_MODIFIER
} TokenTypeTag;

typedef struct {
    const TokenTypeTag ttag;
	union {
        TServiceType service_type;
        TOperatorType operation_type;
        TKeywordType keyword_type;
        TParenType paren_type;
        TDataType data_type;
        TCollectionType collections_type;
        TModifierType modifire_type;
	};
	const char *literal;
} Token;

typedef struct {
    char *input;
    char currch;
    int pos;
    int nextpos;
} Lexer;
