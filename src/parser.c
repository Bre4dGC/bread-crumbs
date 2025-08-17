#include <string.h>

#include "parser.h"
#include "errors.h"

typedef struct ast_node* (*ParseFunction)(struct parser*);

static struct ast_node* parse_block(struct parser* parser);
static struct ast_node* parse_if(struct parser* parser);
static struct ast_node* parse_while(struct parser* parser);
static struct ast_node* parse_for(struct parser* parser);
static struct ast_node* parse_func(struct parser* parser);
static struct ast_node* parse_array(struct parser* parser);
static struct ast_node* parse_struct(struct parser* parser);
static struct ast_node* parse_union(struct parser* parser);
static struct ast_node* parse_enum(struct parser* parser);
static struct ast_node* parse_match(struct parser* parser);
static struct ast_node* parse_trait(struct parser* parser);
static struct ast_node* parse_try(struct parser* parser);
static struct ast_node* parse_import(struct parser* parser);
static struct ast_node* parse_test(struct parser* parser);
static struct ast_node* parse_fork(struct parser* parser);
static struct ast_node* parse_solve(struct parser* parser);
static struct ast_node* parse_simulate(struct parser* parser);

static ParseFunction parse_table[] = {
    [KW_IF] = parse_if,
    [KW_WHILE] = parse_while,
    [KW_FOR] = parse_for,
    [KW_FUNC] = parse_func,
    [KW_STRUCT] = parse_struct,
    [KW_ENUM] = parse_enum,
    [KW_UNION] = parse_union,
    [KW_MATCH] = parse_match,
    [KW_IMPL] = parse_trait,
    [KW_TRY] = parse_try,
    [KW_TEST] = parse_test,
    [KW_FORK] = parse_fork,
    [KW_SOLVE] = parse_solve,
    [KW_SIMULATE] = parse_simulate,
};

struct parser* new_parser(struct lexer* lexer)
{
    struct parser* parser = malloc(sizeof(struct parser));
    if(!parser) return NULL;

    parser->lexer = lexer;

    parser->tcurrent = next_token(lexer);
    parser->tnext = next_token(lexer);

    return parser;
}

struct ast_node* parse_expr(struct parser* parser)
{
    
}

void free_parser(struct parser* parser)
{
    if(parser){
        free_token(&parser->tcurrent);
        free_token(&parser->tnext);
        free_lexer(parser->lexer);
        free(parser);
    }
}

static struct ast_node* parse_block(struct parser* pars)
{
    struct ast_node* block = new_ast(NODE_BLOCK);
    if(!block) return NULL;

    // expect '{'
    if(!(pars->tcurrent.category == CATEGORY_PAREN && pars->tcurrent.paren == PAR_LBRACE)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        print_error(err);
        free_error(err);
        free_ast(block);
        return NULL;
    }
    free_token(&pars->tcurrent);

    pars->tcurrent = pars->tnext;
    pars->tnext = next_token(pars->lexer);

    while(!(pars->tcurrent.category == CATEGORY_PAREN && pars->tcurrent.paren == PAR_RBRACE)
          && !(pars->tcurrent.category == CATEGORY_SERVICE && pars->tcurrent.service == SERV_EOF)){
        free_token(&pars->tcurrent);
        pars->tcurrent = pars->tnext;
        pars->tnext = next_token(pars->lexer);
    }

    // consume '}'
    if(!(pars->tcurrent.category == CATEGORY_PAREN && pars->tcurrent.paren == PAR_RBRACE)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        print_error(err);
        free_error(err);
        free_ast(block);
        return NULL;
    }
    free_token(&pars->tcurrent);

    pars->tcurrent = pars->tnext;
    pars->tnext = next_token(pars->lexer);

    return block;
}

static struct ast_node* parse_if(struct parser* pars)
{
    struct ast_node* if_node = new_ast(NODE_IF);
    if(!if_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_while(struct parser* pars)
{
    struct ast_node* while_node = new_ast(NODE_WHILE);
    if(!while_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_for(struct parser* pars)
{

    struct ast_node* for_node = new_ast(NODE_FOR);
    if(!for_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_func(struct parser* pars)
{

    struct ast_node* func_node = new_ast(NODE_FUNC);
    if(!func_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_array(struct parser* pars)
{

    struct ast_node* array_node = new_ast(NODE_ARRAY);
    if(!array_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_struct(struct parser* pars)
{

    struct ast_node* struct_node = new_ast(NODE_STRUCT);
    if(!struct_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_union(struct parser* pars)
{

    struct ast_node* union_node = new_ast(NODE_UNION);
    if(!union_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_enum(struct parser* pars)
{

    struct ast_node* enum_node = new_ast(NODE_ENUM);
    if(!enum_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_match(struct parser* parser)
{

    struct ast_node* match_node = new_ast(NODE_MATCH);
    if(!match_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_trait(struct parser* pars)
{

    struct ast_node* trait_node = new_ast(NODE_TRAIT);
    if(!trait_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_try(struct parser* pars)
{

    struct ast_node* trycatch_node = new_ast(NODE_TRYCATCH);
    if(!trycatch_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_import(struct parser* pars)
{

    struct ast_node* import_node = new_ast(NODE_IMPORT);
    if(!import_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_test(struct parser* pars)
{

    struct ast_node* test_node = new_ast(NODE_TEST);
    if(!test_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_fork(struct parser* parser)
{

    struct ast_node* fork_node = new_ast(NODE_FORK);
    if(!fork_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_solve(struct parser* pars)
{

    struct ast_node* solve_node = new_ast(NODE_SOLVE);
    if(!solve_node) return NULL;
    return NULL; 
}

static struct ast_node* parse_simulate(struct parser* pars)
{

    struct ast_node* simulate_node = new_ast(NODE_SIMULATE);
    if(!simulate_node) return NULL;
    return NULL; 
}
