#pragma once

#include <stddef.h>
#include <wchar.h>

typedef enum {
	T_ILLEGAL,    // unknown
	T_IDENT,      // identificator
    T_COMMENT,    // comment
	T_EOF,        // end of file
} TServiceType;

typedef enum {
    T_PLUS, T_MINUS, T_ASTERISK, T_SLASH, // + - * /
    T_DOT, T_COMMA, // . ,
    T_LANGLE, T_RANGLE, // <>
    T_PERCENT,  // %
    T_ASSIGN,   // =
    T_COLON,    // :
    T_NOT, T_QUESTION,   // ! ?
    T_RANGE,    // ..
    T_RETTYPE,  // ->
    T_AND, T_OR, // && ||
    T_EQ, T_NEQ, // == !=
    T_LTE, T_GTE, // <= >=
    T_INCREM, T_DECREM, // ++ --
    T_ADD, T_SUB, T_MUL, T_DIV, // += -= *= /=
} TOperatorType;

typedef enum {
    T_IF,       // if
    T_ELIF,     // elif
    T_ELSE,     // else
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
    T_ASYNC,    // async
    T_AWAIT,    // await
    T_TEST,     // test
    T_ASSERT,   // assert
    T_VERIFY,   // verify
    T_WHERE,    // where
    T_SOLVE,    // solve
    T_SNAPSHOT, // snapshot
    T_ROLLBACK, // rollback
    T_COMMIT,   // commit
    T_FORK,     // fork
    T_MERGE,    // merge
    T_REVERT,   // revert
    T_PUSH,     // push
    T_PULL,     // pull
    T_CLONE,    // clone
    T_SIMULATE, // simulate
    T_TIMELINE, // timeline
    T_CHOOSE,   // choose
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
    T_VOID, T_BOOL, T_STR, 
    T_UNI, T_TENSOR, 

    /* exact types */
    T_INT8,    T_INT16,  T_INT32,  T_INT64,
    T_UINT8,   T_UINT16, T_UINT32, T_UINT64,
    T_FLOAT32, T_FLOAT64,
} TDataType;

typedef enum {
    T_NUMBER, T_DECIMAL, T_STRING, T_BOOL_TRUE, T_BOOL_FALSE, T_HEX, T_BIN, T_NULL,
} TValueType;

typedef enum {
    T_VAR,      // var
    T_CONST,    // const
    T_FINAL,    // final 
    T_STATIC,   // static
    T_EVENT,    // event
    T_SIGNAL,   // signal
} TModifierType;

typedef enum {
    TYPE_SERVICE,    // TServiceType
    TYPE_OPERATOR,   // TOperatorType
    TYPE_KEYWORD,    // TKeywordType
    TYPE_PAREN,      // TParenType
    TYPE_DELIMITER,  // TDelimiterType
    TYPE_DATATYPE,   // TDataType
    TYPE_VALUE,      // TValueType
    TYPE_MODIFIER,   // TModifierType
} T_TypeTag;

typedef enum {
    TYPE_WARNING, TYPE_ERROR, TYPE_FATAL,
} ErrorType;

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
        TValueType value;
        TModifierType modifier;
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
