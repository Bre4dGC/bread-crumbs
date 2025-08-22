#include <string.h>

#include "parser.h"

typedef struct ast_node* (*parse_funcions)(struct parser*);

static struct ast_node* parse_block(struct parser* pars);
static struct ast_node* parse_func_call(struct parser* pars);
static struct ast_node* parse_var_decl(struct parser* pars);
static struct ast_node* parse_var_ref(struct parser* pars);
static struct ast_node* parse_unary_op(struct parser* pars);
static struct ast_node* parse_bin_op(struct parser* pars);
static struct ast_node* parse_return(struct parser* pars);
static struct ast_node* parse_if(struct parser* pars);
static struct ast_node* parse_while(struct parser* pars);
static struct ast_node* parse_for(struct parser* pars);
static struct ast_node* parse_func(struct parser* pars);
static struct ast_node* parse_array(struct parser* pars);
static struct ast_node* parse_struct(struct parser* pars);
static struct ast_node* parse_union(struct parser* pars);
static struct ast_node* parse_enum(struct parser* pars);
static struct ast_node* parse_match(struct parser* pars);
static struct ast_node* parse_trait(struct parser* pars);
static struct ast_node* parse_try_catch(struct parser* pars);
static struct ast_node* parse_import(struct parser* pars);
static struct ast_node* parse_test(struct parser* pars);
static struct ast_node* parse_fork(struct parser* pars);
static struct ast_node* parse_solve(struct parser* pars);
static struct ast_node* parse_simulate(struct parser* pars);

static parse_funcions parse_table[] = {
    [KW_IF] = parse_if,
    [KW_WHILE] = parse_while,
    [KW_FOR] = parse_for,
    [KW_FUNC] = parse_func,
    [KW_STRUCT] = parse_struct,
    [KW_ENUM] = parse_enum,
    [KW_UNION] = parse_union,
    [KW_MATCH] = parse_match,
    [KW_IMPL] = parse_trait,
    [KW_TRY] = parse_try_catch,
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

    parser->tok_current = next_token(lexer);
    parser->tok_next = next_token(lexer);

    return parser;
}

static void new_parser_error(struct parser* pars, struct error* err)
{
    if (!pars || !err) {
        if (err) free_error(err);
        return;
    }

    if (!pars->errors) {
        pars->errors = (struct error**)malloc(sizeof(struct error*));
        if (!pars->errors) {
            free_error(err);
            return;
        }
        pars->errors[0] = err;
        pars->errors_count = 1;
        return;
    }

    struct error** new_errors = (struct error**)realloc(
        pars->errors, 
        (pars->errors_count + 1) * sizeof(struct error*)
    );
    if (!new_errors) {
        free_error(err);
        return;
    }
    
    pars->errors = new_errors;
    pars->errors[pars->errors_count] = err;
    pars->errors_count++;
}

void free_parser(struct parser* parser)
{
    if(parser){
        free_token(&parser->tok_current);
        free_token(&parser->tok_next);
        free_lexer(parser->lexer);
        free(parser);
    }
}


struct ast_node* parse_expr(struct parser* parser)
{
    if(!parser) return NULL;

    struct ast_node* node = NULL;

    switch(parser->tok_current.category){
        case CATEGORY_PAREN:
            if(parser->tok_current.paren == PAR_LPAREN){
                node = parse_func_call(parser);
            }
            else if(parser->tok_current.paren == PAR_LBRACE){
                node = parse_block(parser);
            }
            else if(parser->tok_current.paren == PAR_LBRACKET){
                node = parse_array(parser);
            }
            else {
                struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                       parser->lexer->line, parser->lexer->column, 1, parser->lexer->input);
                new_parser_error(parser, err);
                free_error(err);
            }
            break;
        case CATEGORY_LITERAL:
            if(parser->tok_current.value == LIT_IDENT){
                // could be variable declaration or reference
                if(parser->tok_next.category == CATEGORY_OPERATOR && parser->tok_next.oper == OP_ASSIGN){
                    node = parse_var_decl(parser);
                } else {
                    node = parse_var_ref(parser);
                }
            } else {
                struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                       parser->lexer->line, parser->lexer->column, 1, parser->lexer->input);
                new_parser_error(parser, err);
                free_error(err);
            }
            break;
        case CATEGORY_OPERATOR:
            // could be unary or binary operator
            if(parser->tok_next.category == CATEGORY_LITERAL || 
               (parser->tok_next.category == CATEGORY_PAREN && 
               (parser->tok_next.paren == PAR_LPAREN || parser->tok_next.paren == PAR_LBRACE || parser->tok_next.paren == PAR_LBRACKET))){
                node = parse_unary_op(parser);
            } else {
                node = parse_bin_op(parser);
            }
            break;
        case CATEGORY_KEYWORD:
            if(parser->tok_current.keyword == KW_RETURN){
                node = parse_return(parser);
            } else if (parser->tok_current.keyword < sizeof(parse_table)/sizeof(parse_table[0]) && parse_table[parser->tok_current.keyword]) {
                node = parse_table[parser->tok_current.keyword](parser);
            } else {
                struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                       parser->lexer->line, parser->lexer->column, 1, parser->lexer->input);
                new_parser_error(parser, err);
                free_error(err);
            }
            break;
        default:
            struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                    parser->lexer->line, parser->lexer->column, 1, parser->lexer->input);
            new_parser_error(parser, err);
            free_error(err);
            break;
    }
    return node;
}

static struct ast_node* parse_block(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_BLOCK);
    if(!node) return NULL;

    // expect '{'
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_LBRACE)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PAREN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    while(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_RBRACE) &&
          !(pars->tok_current.category == CATEGORY_SERVICE && pars->tok_current.service == SERV_EOF)){
        free_token(&pars->tok_current);
        pars->tok_current = pars->tok_next;
        pars->tok_next = next_token(pars->lexer);
    }

    // consume '}'
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_RBRACE)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    return node;
}

static struct ast_node* parse_func_call(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC_CALL);
    if(!node) return NULL;

    // expect function name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    
    node->func_call.name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // expect '('
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_LPAREN)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PAREN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    // parse arguments
    node->func_call.args = parse_expr(pars);

    // expect ')'
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_RPAREN)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PAREN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    return node;
}

static struct ast_node* parse_var_decl(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR);
    if(!node) return NULL;

    // expect variable name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->simple.var_name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // expect '='
    if(!(pars->tok_current.category == CATEGORY_OPERATOR && pars->tok_current.oper == OP_ASSIGN)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_OPERATOR,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    return node;
}

static struct ast_node* parse_var_ref(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR_REF);
    if(!node) return NULL;

    // expect variable name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->var_ref.name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    return node;
}

static struct ast_node* parse_return(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_RETURN);
    if(!node) return NULL;

    node->return_stmt.body = parse_expr(pars); // parse return value

    return node;
}

static struct ast_node* parse_bin_op(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_BIN_OP);
    if(!node) return NULL;

    // parse left operand
    node->bin_op.left = parse_expr(pars);
    if(!node->bin_op.left) {
        free_ast(node);
        return NULL;
    }

    // expect operator
    if(pars->tok_current.category != CATEGORY_OPERATOR){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_OPERATOR,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->bin_op.code = pars->tok_current.oper; // set operator code
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse right operand
    node->bin_op.right = parse_expr(pars);
    if(!node->bin_op.right) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_unary_op(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_UNARY_OP);
    if(!node) return NULL;

    // expect operator
    if(pars->tok_current.category != CATEGORY_OPERATOR){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_OPERATOR,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->unary_op.code = pars->tok_current.oper; // set operator code
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse operand
    node->unary_op.operand = parse_expr(pars);
    if(!node->unary_op.operand) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_array(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ARRAY);
    if(!node) return NULL;

    // expect '['
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_LBRACKET)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PAREN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse array elements
    node->array_decl->elements = parse_expr(pars);

    // expect ']'
    if(!(pars->tok_current.category == CATEGORY_PAREN && pars->tok_current.paren == PAR_RBRACKET)){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PAREN,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    return node;
}

static struct ast_node* parse_if(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_IF);
    if(!node) return NULL;

    // expect 'if'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_IF){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse condition
    node->if_stmt->condition = parse_expr(pars);
    if(!node->while_loop->condition) {
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse body
    node->if_stmt->body = parse_block(pars);
    if(!node->if_stmt->body) {
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse elif/else
    if(pars->tok_current.category == CATEGORY_KEYWORD && pars->tok_current.keyword == KW_ELIF){
        node->if_stmt->elif_blocks = parse_if(pars);
        if(!node->if_stmt->else_block) {
            free_ast(node);
            return NULL;
        }
    }
    else if(pars->tok_current.category == CATEGORY_KEYWORD && pars->tok_current.keyword == KW_ELSE){
        // parse else body
        node->if_stmt->else_block = parse_block(pars);
        if(!node->if_stmt->else_block) {
            free_ast(node);
            return NULL;
        }
    }
    return node;
}

static struct ast_node* parse_while(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_WHILE);
    if(!node) return NULL;

    // expect 'while'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_WHILE){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse condition
    node->while_loop->condition = parse_expr(pars);
    if(!node->while_loop->condition) {
        free_ast(node);
        return NULL;
    }

    // parse body
    node->while_loop->body = parse_block(pars);
    if(!node->while_loop->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_for(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FOR);
    if(!node) return NULL;

    // expect 'for'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_FOR){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    node->for_loop->init = parse_expr(pars);
    if(!node->for_loop->init) {
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // expect ','
    if(pars->tok_current.category != CATEGORY_OPERATOR || pars->tok_current.keyword != OP_COMMA){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_OPERATOR,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse condition
    node->for_loop->condition = parse_expr(pars);
    if(!node->for_loop->condition) {
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // expect ','
    if(pars->tok_current.category != CATEGORY_OPERATOR || pars->tok_current.keyword != OP_COMMA){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_OPERATOR,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse step
    node->for_loop->step = parse_expr(pars);
    if(!node->for_loop->step) {
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse body
    node->for_loop->body = parse_block(pars);
    if(!node->for_loop->body) {
        free_ast(node);
        return NULL;
    }
    
    return NULL; 
}

static struct ast_node* parse_func(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC);
    if(!node) return NULL;

    // expect 'func'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_FUNC){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse function name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    
    node->func_decl->name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars);

    // parse parameters
    node->func_decl->params = parse_expr(pars); // TODO: implement parameter parsing

    // parse return type
    // node->func_decl->ret_type = parse_expr(pars); // TODO: implement return type parsing

    // parse function body
    node->func_decl->body = parse_block(pars);
    if(!node->func_decl->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_struct(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_STRUCT);
    if(!node) return NULL;

    // expect 'struct'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_STRUCT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse struct name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->struct_decl->name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse struct body
    node->struct_decl->body = parse_block(pars);
    if(!node->struct_decl->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_union(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_UNION);
    if(!node) return NULL;

    // expect 'union'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_UNION){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse union name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->union_decl->name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse union body
    node->union_decl->body = parse_block(pars);
    if(!node->union_decl->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_enum(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ENUM);
    if(!node) return NULL;

    // expect 'enum'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_ENUM){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse enum name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_IDENT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->enum_decl->name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse enum body
    node->enum_decl->body = parse_block(pars);
    if(!node->enum_decl->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_match(struct parser* parser)
{
    struct ast_node* node = new_ast(NODE_MATCH);
    if(!node) return NULL;
    return node;
}

static struct ast_node* parse_trait(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_TRAIT);
    if(!node) return NULL;
    return node; 
}

static struct ast_node* parse_try_catch(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_TRYCATCH);
    if(!node) return NULL;

    // expect 'try'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_TRY){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse condition
    node->trycatch_stmt->condition = parse_expr(pars);
    if(!node->while_loop->condition) {
        free_ast(node);
        return NULL;
    }

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // parse try body
    node->trycatch_stmt->try_block = parse_block(pars);
    if(!node->trycatch_stmt->try_block) {
        free_ast(node);
        return NULL;
    }
    
    // expect 'catch'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_CATCH){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);
    // parse catch body
    node->trycatch_stmt->catch_block = parse_block(pars);
    if(!node->trycatch_stmt->catch_block) {
        free_ast(node);
        return NULL;
    }

    return node; 
}

static struct ast_node* parse_import(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_IMPORT);
    if(!node) return NULL;

    // parse 'import'
    if(pars->tok_current.category != CATEGORY_KEYWORD || pars->tok_current.keyword != KW_IMPORT){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_KEYWORD,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }
    free_token(&pars->tok_current);
    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);

    // expect module name
    if(pars->tok_current.category != CATEGORY_LITERAL || pars->tok_current.value != LIT_STRING){
        struct error* err = new_error(TYPE_FATAL, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_error(err);
        free_ast(node);
        return NULL;
    }

    node->import_stmt->module_name = strdup(pars->tok_current.literal);
    free_token(&pars->tok_current);

    pars->tok_current = pars->tok_next;
    pars->tok_next = next_token(pars->lexer);
    return node; 
}

static struct ast_node* parse_test(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_TEST);
    if(!node) return NULL;
    return node; 
}

static struct ast_node* parse_fork(struct parser* parser)
{
    struct ast_node* node = new_ast(NODE_FORK);
    if(!node) return NULL;
    return node; 
}

static struct ast_node* parse_solve(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_SOLVE);
    if(!node) return NULL;
    return node; 
}

static struct ast_node* parse_simulate(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_SIMULATE);
    if(!node) return NULL;
    return node; 
}