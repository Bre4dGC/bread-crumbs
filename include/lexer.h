#pragma once
#include <stddef.h>
#include <wchar.h>

typedef enum {
	T_ILLEGAL,  // unknown
	T_IDENT,    // identificator
	T_EOF,      // end of file
} TServiceType;

typedef enum {
    T_PLUS,     // +
    T_MINUS,    // -
    T_ASTERISK, // *
    T_SLASH,    // /
    T_PERCENT,  // %
    T_ASSIGN,   // =
    T_DOT,      // .
    T_COMMA,    // ,
    T_COLON,    // :
    T_LANGLE,   // <
    T_RANGLE,   // >
    T_NOT,      // !
    T_AND,      // &&
    T_OR,       // ||
    T_EQ,       // ==
    T_NEQ,      // !=
    T_LTE,      // <=
    T_GTE,      // >=
    T_INCREM,   // ++
    T_DECREM,   // --
    T_ADD,      // +=
    T_SUB,      // -=
    T_MUL,      // *=
    T_DIV,      // /=
    T_RANGE,    // ..
    T_RETTYPE,  // ->
} TOperatorType;

typedef enum {
    T_IF,       // if
    T_ELSE,     // else
    T_ELIF,     // elif
    T_FOR,      // for
    T_DO,       // do
    T_WHILE,    // while
    T_FUNC,     // func
    T_RETURN,   // return
    T_BREAK,    // break
    T_CONTINUE, // continue
    T_DEFAULT,  // default
    T_MATCH,    // match
    T_CASE,     // case
    T_STRUCT,   // struct
    T_ENUM,     // enum
    T_UNION,    // union
    T_IMPL,     // impl
    T_IMPORT,   // import
    T_TYPE,     // type
    T_TRAIT,    // trait
    T_TRY,      // try
    T_CATCH,    // catch
} TKeywordType;

typedef enum {
    T_LPAREN, T_RPAREN,     // ()
    T_LBRACE, T_RBRACE,     // {}
    T_LBRACKET, T_RBRACKET, // []
} TParenType;

typedef enum {
    T_QUOTE,    // "
    T_SQUOTE,   // '
} TDelimiterType;

typedef enum {
    /* basic types */
    T_INT, T_UINT, T_FLOAT,
    T_VOID, T_BOOL, T_STR, T_UNI, T_TENSOR,

    /* exact types */
    T_INT8,    T_INT16,  T_INT32,  T_INT64,
    T_UINT8,   T_UINT16, T_UINT32, T_UINT64,
    T_FLOAT32, T_FLOAT64,
} TDataType;

typedef enum {
    T_VAR, T_CONST, T_FINAL, T_STATIC, T_PUBLIC, T_PRIVATE
} TModifierType;

typedef enum {
    T_LIST, T_STACK, T_MAP, T_VECTOR, T_TUPLE, T_ARRAY, T_SET
} TCollectionType;

typedef enum {
    TYPE_SERVICE,    // TServiceType
    TYPE_OPERATOR,   // TOperatorType
    TYPE_KEYWORD,    // TKeywordType
    TYPE_PAREN,      // TParenType
    TYPE_DELIMITER,  // TDelimiterType
    TYPE_DATATYPE,   // TDataType
    TYPE_MODIFIER,   // TModifierType
    TYPE_COLLECTION, // TCollectionType
} T_TypeTag;

typedef struct {
    wchar_t *literal;
    int type;
    int precedence;
} Operator;

typedef struct {
    wchar_t *literal;
    int type;
} Keyword;

typedef struct {
    T_TypeTag tag;
	union {
        TServiceType service;
        TOperatorType oper;
        TKeywordType keyword;
        TParenType paren;
        TDelimiterType delim;
        TDataType dtype;
        TModifierType modifier;
        TCollectionType collection;
	};
	wchar_t *literal;
} Token;

typedef struct {
    wchar_t *input;
    wchar_t ch;
    size_t pos;
    size_t nextpos;
    size_t line;
    size_t column;
} Lexer;

Lexer* lex_new(const wchar_t *input);
Token tok_new(const T_TypeTag tag, const int value, const wchar_t *literal);
Token tok_next(Lexer *lexer);

void lex_free(Lexer *lexer);
void tok_free(Token *token);

static Token handle_oper(Lexer *lexer);
static Token handle_paren(Lexer *lexer);
static Token handle_num(Lexer *lexer);
static Token handle_ident(Lexer *lexer);
static Token handle_str(Lexer *lexer);

static wchar_t* read_ident(Lexer *lexer);
static wchar_t* read_num(Lexer *lexer);
static wchar_t* read_str(Lexer *lexer);
