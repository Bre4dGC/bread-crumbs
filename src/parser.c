#include <string.h>

#include "parser.h"

typedef ASTNode* (*ParseFunction)(Parser*);

ParseFunction parse_table[] = {
    [T_IF] = parse_if, 
    [T_WHILE] = parse_while,
    [T_FOR] = parse_for,
    [T_FUNC] = parse_func,
    [T_STRUCT] = parse_struct,
    [T_ENUM] = parse_enum,
    [T_UNION] = parse_union,
    [T_MATCH] = parse_match,
    [T_IMPL] = parse_trait,
    [T_TRY] = parse_try,
    [T_TEST] = parse_test,
    [T_FORK] = parse_fork,
    [T_SOLVE] = parse_solve,
    [T_SIMULATE] = parse_simulate,
};

Parser* parser_new(Lexer* lexer)
{
    Parser *parser = malloc(sizeof(Parser));
    if(!parser) return NULL;

    parser->lexer = lexer;

    parser->tcurrent = tok_next(lexer);
    parser->tnext = tok_next(lexer);

    return parser;
}

ASTNode* parse_expr(Parser* parser)
{
    if(parser->tcurrent.tag == TYPE_KEYWORD){
        ParseFunction func = parse_table[parser->tcurrent.keyword];
        if(func){
            return func(parser);
        }
    }
    return parse_block(parser);
}

void parser_free(Parser* parser)
{
    if(parser){
        tok_free(&parser->tcurrent);
        tok_free(&parser->tnext);
        lex_free(parser->lexer);
        free(parser);
    }
}

static ASTNode* parse_block(Parser* parser){}

static ASTNode* parse_if(Parser* parser){}

static ASTNode* parse_while(Parser* parser){}

static ASTNode* parse_for(Parser* parser){}

static ASTNode* parse_func(Parser* parser){}

static ASTNode* parse_array(Parser* parser){}

static ASTNode* parse_struct(Parser* parser){}

static ASTNode* parse_union(Parser* parser){}

static ASTNode* parse_enum(Parser* parser){}

static ASTNode* parse_match(Parser* parser){}

static ASTNode* parse_trait(Parser* parser){}

static ASTNode* parse_try(Parser* parser){}

static ASTNode* parse_test(Parser* parser){}

static ASTNode* parse_fork(Parser* parser){}

static ASTNode* parse_solve(Parser* parser){}

static ASTNode* parse_simulate(Parser* parser){}

void vm_exec(VM* vm){}
