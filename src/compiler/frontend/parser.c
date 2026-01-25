#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/core/string_pool.h"
#include "compiler/frontend/tokenizer.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/parser.h"
#include "common/debug.h"

node_t* parse_expr(parser_t* parser);
node_t* parse_operator_expr(parser_t* parser);
node_t* parse_keyword_expr(parser_t* parser);
node_t* parse_paren_expr(parser_t* parser);
node_t* parse_literal_expr(parser_t* parser);

node_t* parse_stmt(parser_t* parser);
node_t* parse_primary(parser_t* parser);

node_t* parse_block(parser_t* parser);
bool add_block_stmt(parser_t* parser, node_t* block, node_t* stmt);

node_t* parse_postfix(parser_t* parser);
node_t* parse_unary_op(parser_t* parser);
node_t* parse_bin_op(parser_t* parser, int min_preced);

node_t* parse_func_call(parser_t* parser);
node_t* parse_var_decl(parser_t* parser);
node_t* parse_var_ref(parser_t* parser);
node_t* parse_jump_stmt(parser_t* parser);
node_t* parse_special(parser_t* parser);

node_t* parse_if(parser_t* parser);
node_t* parse_while(parser_t* parser);
node_t* parse_for(parser_t* parser);
node_t* parse_func(parser_t* parser);
node_t* parse_array(parser_t* parser);
node_t* parse_struct(parser_t* parser);
node_t* parse_enum(parser_t* parser);
node_t* parse_match(parser_t* parser);

node_t* parse_trait(parser_t* parser);
node_t* parse_type(parser_t* parser);
node_t* parse_impl(parser_t* parser);
node_t* parse_trycatch(parser_t* parser);
node_t* parse_module(parser_t* parser);
node_t* parse_import(parser_t* parser);
node_t* parse_test(parser_t* parser);
node_t* parse_fork(parser_t* parser);
node_t* parse_solve(parser_t* parser);
node_t* parse_simulate(parser_t* parser);

typedef node_t* (*parse_func_t)(parser_t*);

parse_func_t parse_table[] = {
    [KW_IF]       = parse_if,
    [KW_WHILE]    = parse_while,
    [KW_FOR]      = parse_for,
    [KW_FUNC]     = parse_func,
    [KW_STRUCT]   = parse_struct,
    [KW_ENUM]     = parse_enum,
    [KW_MATCH]    = parse_match,
    
    [KW_TYPE]     = parse_type,
    [KW_MODULE]   = parse_module,
    [KW_IMPORT]   = parse_import,
    [KW_TRAIT]    = parse_trait,
    [KW_IMPL]     = parse_impl,
    [KW_TRY]      = parse_trycatch,
    [KW_TEST]     = parse_test,
    [KW_FORK]     = parse_fork,
    [KW_SOLVE]    = parse_solve,
    [KW_SIMULATE] = parse_simulate,
    
    [KW_RETURN]   = parse_jump_stmt,
    [KW_BREAK]    = parse_jump_stmt,
    [KW_CONTINUE] = parse_jump_stmt,

    [KW_NAMEOF]    = parse_special,
    [KW_TYPEOF]    = parse_special,
};

parser_t* new_parser(arena_t* arena, arena_t* ast, report_table_t* reports, string_pool_t* string_pool, lexer_t* lexer)
{
    parser_t* parser = (parser_t*)arena_alloc(arena, sizeof(parser_t), alignof(parser_t));
    if(!parser) return NULL;
    parser->token.current = next_token(lexer);
    parser->token.next = next_token(lexer);
    parser->lexer = lexer;
    parser->ast = ast;
    parser->reports = reports;
    parser->string_pool = string_pool;
    return parser;
}

bool check_token(parser_t* parser, enum category_tag category, int type)
{
    return parser->token.current.category == category && parser->token.current.type == type;
}

bool is_eof(const token_t token)
{
    return token.category == CAT_SERVICE && token.type == SERV_EOF;
}

void advance_token(parser_t* parser)
{
    if(!parser || is_eof(parser->token.next)) return;    
    parser->token.current = parser->token.next;
    parser->token.next = next_token(parser->lexer);
}

bool consume_token(parser_t* parser, const enum category_tag expec_category, const int expec_type, const enum report_code err)
{
    if(!parser) return false;
    
    if(parser->token.current.category != expec_category){
        add_report(parser->reports, SEV_ERR, err, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return false;
    }

    if(expec_type >= 0){
        int actual_type = parser->token.current.type;

        if(actual_type != expec_type){
            add_report(parser->reports, SEV_ERR, ERR_UNEXP_TOKEN, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return false;
        }
    }

    advance_token(parser);
    return true;
}

ast_t* parse_program(parser_t* parser)
{
    if(!parser) return NULL;

    ast_t* ast = (ast_t*)arena_alloc(parser->ast, sizeof(ast_t), alignof(ast_t));
    if (!ast) return NULL;

    ast->nodes = new_node(parser->ast, NODE_BLOCK);
    ast->count = 0;

    while(!is_eof(parser->token.next)){
        token_t prev_token = parser->token.current;
        
        node_t* stmt = parse_stmt(parser);
        if(!stmt){
            if((parser->token.current.category == prev_token.category && 
                parser->token.current.type == prev_token.type) &&
               !is_eof(parser->token.current) && !is_eof(parser->token.next)){
                advance_token(parser);
            }
            continue;
        }

        if(!add_block_stmt(parser, ast->nodes, stmt)) goto cleanup;

        // optionally consume ';'
        if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
            advance_token(parser);
        }

        ast->count++;
    }

    #ifdef DEBUG
    print_ast(ast->nodes, 0);
    #endif

    return ast;

cleanup:
    free_ast(parser->ast);
    return NULL;
}

node_t* parse_expr(parser_t* parser)
{
    if(!parser) return NULL;

    switch(parser->token.current.category){
        case CAT_OPERATOR: return parse_operator_expr(parser);
        case CAT_KEYWORD:  return parse_keyword_expr(parser);
        case CAT_PAREN:    return parse_paren_expr(parser);
        case CAT_MODIFIER: return parse_var_decl(parser);
        case CAT_LITERAL:  return parse_bin_op(parser, 0);
        default: return NULL;
    }

    return NULL;
}

node_t* parse_keyword_expr(parser_t* parser)
{
    const int kw = parser->token.current.type;
    
    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];

    if(!func){
        return NULL;
    }
    
    return func(parser);
}

node_t* parse_paren_expr(parser_t* parser)
{
    switch(parser->token.current.type){
        case PAR_LBRACE:   return parse_block(parser);            
        case PAR_LBRACKET: return parse_array(parser);            
        case PAR_LPAREN:
            advance_token(parser);
            
            node_t* node = parse_expr(parser);
            if(!node) return NULL;
            
            if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
                return NULL;
            }
            return node;
        default: return NULL;
    }
}

node_t* parse_literal_expr(parser_t* parser)
{
    if(parser->token.current.type == LIT_IDENT){
        if(parser->token.next.category == CAT_OPERATOR && parser->token.next.type == OPER_ASSIGN){
            return parse_var_decl(parser);
        }
        if(parser->token.next.category == CAT_PAREN && parser->token.next.type == PAR_LPAREN){
            return parse_func_call(parser);
        }        
        return parse_var_ref(parser);
    }
    
    node_t* node = new_node(parser->ast, NODE_LITERAL);
    if(!node) return NULL;

    node->lit->value = new_string(parser->string_pool, parser->token.current.literal);// parser->token.current.literal ? FIX
    node->lit->type = parser->token.current.type;
    
    advance_token(parser);
    return node;
}

node_t* parse_operator_expr(parser_t* parser)
{
    bool is_unary = (parser->token.next.category == CAT_LITERAL) ||
                    (parser->token.next.category == CAT_PAREN    &&
                    (parser->token.next.type     == PAR_LPAREN   ||
                     parser->token.next.type     == PAR_LBRACE   ||
                     parser->token.next.type     == PAR_LBRACKET));
    
    return is_unary ? parse_unary_op(parser) : parse_bin_op(parser, 0);
}

node_t* parse_keyword_stmt(parser_t* parser)
{
    int kw = parser->token.current.type;

    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];
    if(!func){
        return NULL;
    }
    
    return func(parser);
}

node_t* parse_stmt(parser_t* parser)
{
    if(!parser) return NULL;
    
    // skip empty statements
    if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        advance_token(parser);
        return NULL;
    }
    
    if(parser->token.current.category == CAT_KEYWORD){
        return parse_keyword_stmt(parser);
    }
    
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        return parse_block(parser);
    }
    
    return parse_expr(parser);
}

bool add_block_stmt(parser_t* parser, node_t* node, node_t* stmt)
{
    if(node->kind != NODE_BLOCK) return false;
    
    if(node->block->statement.count >= node->block->statement.capacity){
        size_t new_capacity = node->block->statement.capacity == 0 ? 4 : node->block->statement.capacity * 2;
        node_t** new_statements = (node_t**)arena_alloc_array(parser->ast, sizeof(node->block->statement.elems), new_capacity * sizeof(node_t*), alignof(node_t*));
        if(!new_statements) return false;
        
        node->block->statement.elems = new_statements;
        node->block->statement.capacity = new_capacity;
    }
    
    node->block->statement.elems[node->block->statement.count++] = stmt;
    return true;
}

node_t* parse_block(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_BLOCK);
    if(!node) return NULL;
        
    advance_token(parser); // skip '{'
    
    // parse statements until '}'
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        
        token_t prev_token = parser->token.current;
        
        // parse statement
        node_t* stmt = parse_stmt(parser);
        if(!stmt){
            if(parser->token.current.category == prev_token.category
            && parser->token.current.type == prev_token.type 
            && !check_token(parser, CAT_PAREN, PAR_RBRACE))
            {
                advance_token(parser);
            }
            continue;
        }
        
        // add to block
        if(!add_block_stmt(parser, node, stmt)) return NULL;
        
        // optionally consume ';'
        if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
            advance_token(parser);
        }
    }
    
    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_func_call(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_CALL);
    if(!node) return NULL;

    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        return NULL;
    }
    node->call->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->call->name.data) return NULL;
    advance_token(parser);

    size_t capacity = 0;
    if(check_token(parser, CAT_PAREN, PAR_LPAREN)){
        advance_token(parser); // skip '('
        while (!check_token(parser,  CAT_PAREN, PAR_RPAREN)){
            node_t* arg = parse_expr(parser);
            if(!arg) return NULL;

            if(node->call->args.count >= capacity){
                size_t new_capacity = capacity == 0 ? 4 : capacity * 2;
                node_t** new_args = (node_t**)arena_alloc_array(parser->ast, sizeof(node->call->args), new_capacity * sizeof(node_t*), alignof(node_t*));
                if(!new_args) return NULL;
                node->call->args.elems = new_args;
                capacity = new_capacity;
            }

            node->call->args.elems[node->call->args.count++] = arg;

            if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
                advance_token(parser); // skip ','
            }
            else {
                break;
            }
        }

        if(!check_token(parser,  CAT_PAREN, PAR_RPAREN)){
            return NULL;
        }
        advance_token(parser); // skip ')'
    }

    return node;
}

node_t* parse_var_decl(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_VAR);
    if(!node) return NULL;

    // optional modifier
    if(parser->token.current.category == CAT_MODIFIER){
        node->var_decl->modif = parser->token.current.type;
        advance_token(parser);
    }
    
    // expect identifier
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->var_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->var_decl->name.data) return NULL;

    advance_token(parser);

    // optional type annotation
    if(check_token(parser, CAT_OPERATOR, OPER_COLON)){
        advance_token(parser);
        if(parser->token.current.category !=  CAT_DATATYPE){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_TYPE, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        node->var_decl->dtype = parser->token.current.type;
        advance_token(parser);
    }

    // optional assignment
    if(check_token(parser, CAT_OPERATOR, OPER_ASSIGN)){
        advance_token(parser);
        node->var_decl->value = parse_expr(parser);
        if(!node->var_decl->value){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_EXPR, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
    }

    return node;
}

node_t* parse_var_ref(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_REF);
    if(!node) return NULL;
    
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }

    node->var_ref->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->var_ref->name.data) return NULL;

    advance_token(parser);

    return node;
}

node_t* parse_jump_stmt(parser_t* parser)
{
    node_t* node = new_node(parser->ast, 0);
    if(!node) return NULL;

    int type = parser->token.current.type;

    advance_token(parser); // skip

    switch(type){
        case KW_BREAK:
            node->kind = NODE_BREAK;
            break;
        case KW_CONTINUE:
            node->kind = NODE_CONTINUE;
            break;
        case KW_RETURN:
            node->kind = NODE_RETURN;
            node->ret->body = parse_expr(parser);
            if(!node->ret->body) return NULL;
            break;
    }

    return node;
}

node_t* parse_primary(parser_t* parser)
{
    switch(parser->token.current.category){
        case CAT_LITERAL:
            if(parser->token.current.type == LIT_IDENT){
                if(parser->token.next.category == CAT_PAREN && parser->token.next.type == PAR_LPAREN){
                    return parse_func_call(parser);
                }
                return parse_var_ref(parser);
            }
            else {
                node_t* node = new_node(parser->ast, NODE_LITERAL);
                if(!node) return NULL;
                node->lit->value = new_string(parser->string_pool, parser->token.current.literal);
                node->lit->type = parser->token.current.type;
                advance_token(parser);
                return node;
            }

        case CAT_PAREN:
            if(parser->token.current.type == PAR_LPAREN){
                advance_token(parser);
                node_t* expr = parse_expr(parser);
                if(!expr) return NULL;
                if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
                    return NULL;
                }
                return expr;
            }
            else if(parser->token.current.type == PAR_LBRACKET){
                return parse_array(parser);
            }
            break;
        case CAT_OPERATOR:
            // check for unary operators
            if(parser->token.current.type == OPER_PLUS  ||
               parser->token.current.type == OPER_MINUS ||
               parser->token.current.type == OPER_NOT)
            {
                return parse_unary_op(parser);
            }
            break;
        default: break;
    }
    return NULL;

}

int get_operator_precedence(enum category_operator op)
{
    switch(op){
        case OPER_INCREM: 
        case OPER_DECREM:
            return 15; 
        
        case OPER_ASTERISK:
        case OPER_SLASH:
        case OPER_PERCENT:
            return 13;
    
        case OPER_PLUS:
        case OPER_MINUS:
            return 12;
        
        case OPER_LANGLE:
        case OPER_RANGLE:
        case OPER_LTE:
        case OPER_GTE:
            return 10;
        
        case OPER_EQ:
        case OPER_NEQ:
            return 9;

        case OPER_AND:
            return 5;

        case OPER_OR:
            return 4;

        case OPER_ASSIGN:
        case OPER_ADD:
        case OPER_SUB:
        case OPER_MUL:
        case OPER_DIV:
        case OPER_MOD:
            return 2;

        case OPER_COMMA:
            return 1;
        
        case OPER_DOT:
        case OPER_ARROW:
        case OPER_SEMICOLON:
        case OPER_COLON:
        case OPER_QUESTION:
        case OPER_RANGE:
        case OPER_NOT:
        default:
            return 0;
    }
}

bool is_right_associative(enum category_operator op)
{
    switch(op){
        case OPER_ASSIGN: case OPER_ADD: case OPER_SUB: case OPER_MUL: case OPER_DIV: case OPER_MOD:
            return true;
        default:
            return false;
    }
}

bool is_binary_operator(enum category_operator op)
{
    int precedence = get_operator_precedence(op);
    return precedence > 0 && op != OPER_SEMICOLON && op != OPER_COMMA;
}

node_t* parse_postfix(parser_t* parser)
{
    node_t* expr = parse_primary(parser);
    if(!expr) return NULL;

    while(parser->token.current.category == CAT_OPERATOR){
        int op = parser->token.current.type;

        if(op == OPER_INCREM || op == OPER_DECREM){
            node_t* postfix = new_node(parser->ast, NODE_UNARYOP);
            if(!postfix){
                return NULL;
            }
            
            postfix->unaryop->right = expr;
            postfix->unaryop->is_postfix = true;
            
            advance_token(parser);
            expr = postfix;
        }
        else break;
    }
    return expr;
}

node_t* parse_bin_op(parser_t* parser, int min_precedence)
{
    node_t* left = parse_postfix(parser);
    if(!left) return NULL;

    while(parser->token.current.category == CAT_OPERATOR){
        enum category_operator op = parser->token.current.type;        
        if(op == OPER_SEMICOLON || op == OPER_COMMA) break;
        
        int precedence = get_operator_precedence(op);        
        if(precedence == 0 || precedence < min_precedence) break;

        advance_token(parser);

        // calculate next minimum precedence
        int next_min_prec;
        if(is_right_associative(op)) next_min_prec = precedence;
        else next_min_prec = precedence + 1;

        node_t* right = parse_bin_op(parser, next_min_prec);
        if(!right){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_EXPR, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        node_t* node = new_node(parser->ast, NODE_BINOP);
        if(!node) return NULL;

        node->binop->left = left;
        node->binop->right = right;
        node->binop->operator = op;
    #ifdef DEBUG
        node->binop->lit = parser->token.current.literal;
    #endif

        left = node;
    }

    return left;
}

node_t* parse_unary_op(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_UNARYOP);
    if(!node) return NULL;

    enum category_operator op_type = parser->token.current.type;
    
    if(op_type != OPER_PLUS
    && op_type != OPER_MINUS
    && op_type != OPER_NOT
    && op_type != OPER_INCREM
    && op_type != OPER_DECREM)
    {
        return NULL;
    }

    node->unaryop->operator = op_type;
    node->unaryop->is_postfix = false;
#ifdef DEBUG
    node->unaryop->lit = parser->token.current.literal;
#endif
    advance_token(parser);

    if(op_type == OPER_INCREM || op_type == OPER_DECREM){
        node->unaryop->right = parse_postfix(parser);
    }
    else {
        node->unaryop->right = parse_primary(parser);
    }
    
    if(!node->unaryop->right) return NULL;

    return node;
}

node_t* parse_array(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_ARRAY);
    if(!node) return NULL;

    advance_token(parser); // skip '['

    // parse elements (comma separated)
    while(!check_token(parser, CAT_PAREN, PAR_RBRACKET)){
        node_t* element = parse_primary(parser);
        if(!element) return NULL;

        if(node->array_decl->count >= node->array_decl->capacity){
            size_t new_cap = node->array_decl->capacity == 0 ? 4 : node->array_decl->capacity * 2;
            node_t** new_arr = (node_t**)arena_alloc_array(parser->ast, sizeof(node->array_decl->elements[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_arr) return NULL;
            node->array_decl->elements = new_arr;
            node->array_decl->capacity = new_cap;
        }
        node->array_decl->elements[node->array_decl->count++] = element;

        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
            advance_token(parser); // consume comma
            continue;
        }
        break;
    }

    // expect ']'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACKET, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_if(parser_t* parser){
    node_t* node = new_node(parser->ast, NODE_IF);
    if(!node) return NULL;

    advance_token(parser); // skip 'if'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    node->if_stmt->condition = parse_expr(parser);
    if(!node->if_stmt->condition) return NULL;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // expect '{'
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->if_stmt->then_block = parse_block(parser);
    }
    else {
        node->if_stmt->then_block = parse_stmt(parser);
    }

    if(!node->if_stmt->then_block) return NULL;

    // parse elif/else blocks
    while(parser->token.current.category == CAT_KEYWORD &&
         (parser->token.current.type == KW_ELIF || parser->token.current.type == KW_ELSE))
    {

        // expect optional 'elif'
        if(parser->token.current.type == KW_ELIF){
            advance_token(parser);

            // expect '('
            if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
                return NULL;
            }

            node_t* elif_condition = parse_expr(parser);
            if(!elif_condition) return NULL;

            // expect ')'
            if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
                return NULL;
            }

            node_t* elif_body;

            // expect '{'
            if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
                elif_body = parse_block(parser);
            }
            else {
                elif_body = parse_stmt(parser);
            }

            if(!elif_body) return NULL;

            node_t* elif_node = new_node(parser->ast, NODE_IF);
            if(!elif_node) return NULL;

            elif_node->if_stmt = (struct node_if*)arena_alloc(parser->ast, sizeof(struct node_if), alignof(struct node_if));
            if(!elif_node->if_stmt) return NULL;

            elif_node->if_stmt->condition = elif_condition;
            elif_node->if_stmt->then_block = elif_body;
            elif_node->if_stmt->elif_blocks = NULL;
            elif_node->if_stmt->else_block = NULL;

            if(!node->if_stmt->elif_blocks){
                node->if_stmt->elif_blocks = elif_node;
            }
            else {
                node_t* current = node->if_stmt->elif_blocks;
                while(current->if_stmt->elif_blocks){
                    current = current->if_stmt->elif_blocks;
                }
                current->if_stmt->elif_blocks = elif_node;
            }
        }

        // expect optional 'else'
        else if(parser->token.current.type == KW_ELSE){
            advance_token(parser);

            // expect '{'
            if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
                node->if_stmt->else_block = parse_block(parser);
            }
            else {
                node->if_stmt->else_block = parse_stmt(parser);
            }

            if(!node->if_stmt->else_block) return NULL;
            break; 
        }
    }
    return node;
}

node_t* parse_while(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_WHILE);
    if(!node) return NULL;

    advance_token(parser); // skip 'while'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    node->while_loop->condition = parse_expr(parser);
    if(!node->while_loop->condition) return NULL;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->while_loop->body = parse_block(parser);
    }
    else {
        node->while_loop->body = parse_stmt(parser);
    }
    
    if(!node->while_loop->body) return NULL;

    return node;
}

node_t* parse_for(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_FOR);
    if(!node) return NULL;

    advance_token(parser); // skip 'for'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse init statement
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_loop->init = parse_expr(parser);
        if(!node->for_loop->init) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        return NULL;
    }

    // parse condition
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_loop->condition = parse_expr(parser);
        if(!node->for_loop->condition) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        return NULL;
    }

    // parse update statement
    if(!check_token(parser,  CAT_PAREN, PAR_RPAREN)){
        node->for_loop->update = parse_expr(parser);
        if(!node->for_loop->update) return NULL;
    }

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // Parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->for_loop->body = parse_block(parser);
    }
    else {
        node->for_loop->body = parse_stmt(parser);
    }
    
    if(!node->for_loop->body) return NULL;

    return node;
}

node_t* parse_func(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_FUNC);
    if(!node) return NULL;

    advance_token(parser); // skip 'func'

    // expect function name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    
    node->func_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->func_decl->name.data) return NULL;
    advance_token(parser);

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    node->func_decl->param.elems = (node_t**)arena_alloc(parser->ast, node->func_decl->param.capacity * sizeof(node_t*), alignof(node_t*));
    if(!node->func_decl->param.elems) return NULL;

    // parsing params until ')'
    if(!check_token(parser, CAT_PAREN, PAR_RPAREN)){
        while(true){
            node_t* param = parse_var_decl(parser);
            if(!param) return NULL;

            // check if param is a variable
            if(param->kind != NODE_VAR){
                add_report(parser->reports, SEV_ERR, ERR_EXPEC_PARAM, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
                return NULL;
            }

            // check if there is enough capacity
            if(node->func_decl->param.count >= node->func_decl->param.capacity){
                node->func_decl->param.capacity *= 2;
                node_t** new_params = (node_t**)arena_alloc_array(parser->ast, sizeof(node->func_decl->param.elems[0]), node->func_decl->param.capacity * sizeof(node_t*), alignof(node_t));
                if(!new_params) return NULL;
                node->func_decl->param.elems = new_params;
            }

            node->func_decl->param.elems[node->func_decl->param.count++] = param;

            // consume ','
            if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
                advance_token(parser);
                continue;
            }
            break;
        }
    }
    
    advance_token(parser); // consume ')'

    // optional return type
    if(check_token(parser, CAT_OPERATOR, OPER_COLON)){
        advance_token(parser);
        
        // expect datatype
        if(parser->token.current.category != CAT_DATATYPE){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_TYPE, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        
        node->func_decl->return_type = parser->token.current.type;
        advance_token(parser);
    }

    // expect function body (block)
    node->func_decl->body = parse_block(parser);
    if(!node->func_decl->body) return NULL;

    return node;
}

node_t* parse_struct(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_STRUCT);
    if(!node) return NULL;

    advance_token(parser); // skip 'struct'

    // expect struct name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->struct_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse struct members
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE)){
        
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        
        // parse member variable declaration
        node_t* member = parse_var_decl(parser);
        if(!member) return NULL;

        // add member
        if(node->struct_decl->member.count >= node->struct_decl->member.capacity){
            size_t new_cap = node->struct_decl->member.capacity == 0 ? 4 : node->struct_decl->member.capacity * 2;
            node_t** new_members = (node_t**)arena_alloc_array(parser->ast, sizeof(node->struct_decl->member.elems[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_members) return NULL;
            node->struct_decl->member.elems = new_members;
            node->struct_decl->member.capacity = new_cap;
        }
        node->struct_decl->member.elems[node->struct_decl->member.count++] = member;
        
        // optional comma separator
        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
            advance_token(parser);
        }
    }
    
    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_enum(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_ENUM);
    if(!node) return NULL;

    advance_token(parser); // skip 'enum'

    // expect enum name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->enum_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse enum members
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        // expect member name
        char* member_name = NULL;
        if(!member_name) return NULL;
        advance_token(parser);

        // add member
        if(node->enum_decl->member.count >= node->enum_decl->member.capacity){
            size_t new_cap = node->enum_decl->member.capacity == 0 ? 4 : node->enum_decl->member.capacity * 2;
            node_t** new_members = (node_t**)arena_alloc_array(parser->ast, sizeof(node->enum_decl->member.elems[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_members) return NULL;
            node->enum_decl->member.elems = new_members;
            node->enum_decl->member.capacity = new_cap;
        }
        node->enum_decl->name.data = member_name;


        // optional comma separator
        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
            advance_token(parser);
        }
    }

    return node;
}

node_t* parse_match(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_MATCH);
    if(!node) return NULL;

    advance_token(parser); // skip 'match'

    // parse target expression
    node->match_stmt->target = parse_expr(parser);
    if(!node->match_stmt->target) return NULL;

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse cases
    while(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN))
    {
        // check for EOF
        if(!consume_token(parser, CAT_KEYWORD, KW_CASE, ERR_EXPEC_KEYWORD)){
            return NULL;
        }

        // create new case node
        node_t* case_node = new_node(parser->ast, NODE_CASE);
        if(!case_node) return NULL;

        // create case struct
        case_node->match_case = (struct node_case*)arena_alloc(parser->ast, sizeof(struct node_case), alignof(struct node_case));
        if(!case_node->match_case) return NULL;

        // parse case condition 
        case_node->match_case->condition = parse_expr(parser);
        if(!case_node->match_case->condition) return NULL;

        // expect '->'
        if(!consume_token(parser, CAT_OPERATOR, OPER_ARROW, ERR_EXPEC_OPER)){
            return NULL;
        }

        // parse case body (can be a block or a single statement)
        case_node->match_case->body = parse_stmt(parser);
        if(!case_node->match_case->body) return NULL;

        // add case to match statement
        if(node->match_stmt->block.count >= node->match_stmt->block.capacity){
            size_t new_cap = node->match_stmt->block.capacity == 0 ? 4 : node->match_stmt->block.capacity * 2;
            node_t** new_cases = (node_t**)arena_alloc_array(parser->ast, sizeof(node->match_stmt->block.elems[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_cases) return NULL;
            node->match_stmt->block.elems = new_cases;
            node->match_stmt->block.capacity = new_cap;
        }
        node->match_stmt->block.elems[node->match_stmt->block.count++] = case_node;
    }

    return node;
}

node_t* parse_trait(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_TRAIT);
    if(!node) return NULL;

    advance_token(parser); // skip 'trait'

    // expect name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->trait_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body
    node->trait_decl->body = parse_block(parser);
    if(!node->trait_decl->body) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_type(parser_t* parser)
{
    return NULL;
}

node_t* parse_impl(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_IMPL);
    if(!node) return NULL;
    
    advance_token(parser); // skip 'impl'
    
    // expect name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->impl_stmt->trait_name = new_string(parser->string_pool, parser->token.current.literal);

    advance_token(parser);

    if(check_token(parser, CAT_KEYWORD, KW_FOR)){
        advance_token(parser); // skip 'for'
        
        // expect name
        if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        node->impl_stmt->struct_name = new_string(parser->string_pool, parser->token.current.literal);
    }
    advance_token(parser);
    
    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body
    node->impl_stmt->body = parse_block(parser);
    if(!node->impl_stmt->body) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_trycatch(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_TRYCATCH);
    if(!node) return NULL;

    advance_token(parser); // skip 'try'

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse try block
    node->trycatch_stmt->try_block = parse_block(parser);
    if(!node->trycatch_stmt->try_block) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){ 
        return NULL;
    }

    // expect 'catch'
    if(!consume_token(parser, CAT_KEYWORD, KW_CATCH, ERR_EXPEC_KEYWORD)){
        return NULL;
    }

    advance_token(parser);

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // TODO: parse catch exception variable

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse catch block
    node->trycatch_stmt->catch_block = parse_block(parser);
    if(!node->trycatch_stmt->catch_block) return NULL;

    return node;
}

node_t* parse_module(parser_t* parser)
{
    return NULL;
}

node_t* parse_import(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_IMPORT);
    if(!node) return NULL;

    advance_token(parser); // skip 'import'

    // expect module name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->import_decl->path = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);
    
    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse import modules
    for(size_t i = 0; i < 1; ++i){
        // node->import_decl->module.elems[i] = ;
        // if(!node->import_decl->module.elems[i]) return NULL;
    }

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;
}

node_t* parse_test(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_TEST);
    if(!node) return NULL;

    advance_token(parser); // skip 'test'

    // expect name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->test_stmt->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse test block
    node->test_stmt->body = parse_block(parser);
    if(!node->test_stmt->body) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    return node;

}

node_t* parse_fork(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_FORK);
    if(!node) return NULL;

    advance_token(parser); // skip 'fork'

    // expect name
    if(!check_token(parser,  CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->fork_stmt->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    node->fork_stmt->body = parse_block(parser);
    if(!node->fork_stmt->body) return NULL;

    return node;
}

node_t* parse_solve(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_SOLVE);
    if(!node) return NULL;

    advance_token(parser); // skip 'solve'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse parameters
    if(!check_token(parser, CAT_PAREN, PAR_RPAREN)){
        while(true)
        {
            node_t* param = parse_var_decl(parser);
            if(!param) return NULL;

            // check capacity
            if(node->solve_stmt->param.count >= node->solve_stmt->param.capacity){
                size_t new_cap = node->solve_stmt->param.capacity == 0 ? 4 : node->solve_stmt->param.capacity * 2;
                node_t** new_arr = (node_t**)arena_alloc_array(parser->ast, sizeof(node->solve_stmt->param), new_cap * sizeof(node_t*), alignof(node_t*));
                if(!new_arr) return NULL;

                node->solve_stmt->param.elems = new_arr;
                node->solve_stmt->param.capacity = new_cap;
            }
            node->solve_stmt->param.elems[node->solve_stmt->param.count++] = param;

            // consume comma
            if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
                advance_token(parser);
                continue;
            }

            break;
        }
    }

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body
    node->solve_stmt->body = parse_block(parser);
    if(!node->solve_stmt->body) return NULL;

    return node;
}

node_t* parse_simulate(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_SIMULATE);
    if(!node) return NULL;

    advance_token(parser); // skip 'simulate'

    // parse body
    node->simulate_stmt->body = parse_block(parser);
    if(!node->simulate_stmt->body) return NULL;

    return node;
}

node_t* parse_special(parser_t* parser)
{
    int spec_kind = parser->token.current.type;

    node_t* node = new_node(parser->ast, spec_kind);
    if(!node) return NULL;

    advance_token(parser); // skip special keyword
           
    if(!check_token(parser, CAT_OPERATOR, PAR_LPAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_OPER, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
    }

    node->spec_stmt->content = new_string(parser->lexer->string_pool, parser->token.current.literal);

    if(!check_token(parser, CAT_OPERATOR, PAR_RPAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_OPER, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
    }
    return node;
}
