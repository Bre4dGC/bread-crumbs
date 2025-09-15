#include <string.h>
#include <stdio.h>

#include "parser.h"
#include "utils.h"
#define DEBUG
#include "debug.h"

static struct ast_node* parse_block(struct parser* pars);
static struct ast_node* parse_func_call(struct parser* pars);
static struct ast_node* parse_var_decl(struct parser* pars);
static struct ast_node* parse_var_ref(struct parser* pars);
static struct ast_node* parse_unary_op(struct parser* pars);
static struct ast_node* parse_bin_op(struct parser* pars, int min_precedence);
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

typedef struct ast_node* (*parse_func_t)(struct parser*);

static struct parse_func_t* parse_table[] = {
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

    parser->current = next_token(lexer);
    parser->peek = next_token(lexer);
    parser->errors = NULL;
    parser->errors_count = 0;
    parser->line = 1;
    parser->column = 1;

    return parser;
}

static void new_parser_error(struct parser* pars, struct error* err)
{
    if(!pars || !err){
        if(err) free_error(err);
        return;
    }

    if(!pars->errors){
        pars->errors = (struct error**)malloc(sizeof(struct error*));
        if(!pars->errors){
            free_error(err);
            return;
        }
        pars->errors[0] = err;
        pars->errors_count = 1;
        return;
    }

    struct error** new_errors = (struct error**)realloc(pars->errors, (pars->errors_count + 1) * sizeof(struct error*));
    if(!new_errors){
        free_error(err);
        return;
    }
    
    pars->errors = new_errors;
    pars->errors[pars->errors_count] = err;
    pars->errors_count++;
}

void free_parser(struct parser* parser)
{
    if(!parser) return;

    free_token(&parser->current);
    free_token(&parser->peek);

    if(parser->errors){
        for (size_t i = 0; i < parser->errors_count; ++i){
            if(parser->errors[i]) free_error(parser->errors[i]);
        }
        free(parser->errors);
    }

    if(parser->lexer) free_lexer(parser->lexer);
    free(parser);
}

void advance_token(struct parser *pars)
{
    if(!pars || (pars->current.category == CATEGORY_SERVICE && pars->current.type_service == SERV_EOF)) return;

    pars->line = pars->lexer->line;
    pars->column = pars->lexer->column;

    free_token(&pars->current);
    
    pars->current = pars->peek;
    pars->peek = next_token(pars->lexer);
}

bool consume_token(
    const struct parser *pars,
    const enum category_tag expected_category,
    const int expected_type,
    const enum parser_error_type err)
{
    if(!pars) return false;
    
    if(pars->current.category != expected_category){
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, (int)err,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        return false;
    }

    if(expected_type >= 0){
        int actual_type = -1;
        switch (expected_category){
            case CATEGORY_SERVICE:  actual_type = pars->current.type_service;  break;
            case CATEGORY_OPERATOR: actual_type = pars->current.type_operator; break;
            case CATEGORY_KEYWORD:  actual_type = pars->current.type_keyword;  break;
            case CATEGORY_PAREN:    actual_type = pars->current.type_paren;    break;
            case CATEGORY_DELIMITER:actual_type = pars->current.type_delim;    break;
            case CATEGORY_DATATYPE: actual_type = pars->current.type_datatype; break;
            case CATEGORY_LITERAL:  actual_type = pars->current.type_literal;  break;
            case CATEGORY_MODIFIER: actual_type = pars->current.type_modifier; break;
            default: actual_type = -1; break;
        }

        if(actual_type != expected_type){
            struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_UNEXPECTED_TOKEN,
                                   pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
            new_parser_error(pars, err);
            return false;
        }
    }

    advance_token(pars);
    return true;
}

static struct ast_node* parse_operator_expr(struct parser* pars);
static struct ast_node* parse_keyword_expr(struct parser* pars);
static struct ast_node* parse_paren_expr(struct parser* pars);
static struct ast_node* parse_literal_expr(struct parser* pars);

struct ast_node* parse_expr(struct parser* pars)
{
    if(!pars) return NULL;

    switch(pars->current.category){
        case CATEGORY_OPERATOR: return parse_operator_expr(pars);
        case CATEGORY_KEYWORD:  return parse_keyword_expr(pars);
        case CATEGORY_PAREN:    return parse_paren_expr(pars);
        case CATEGORY_LITERAL:  return parse_literal_expr(pars);
        case CATEGORY_MODIFIER: return parse_var_decl(pars);
        default: return NULL;
    }

    return NULL;
}

static struct ast_node* parse_keyword_expr(struct parser* pars)
{
    const int kw = pars->current.type_keyword;
    
    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];

    if(!func){
        fprintf(stderr, "No parser for keyword: %d\n", kw);
        return NULL;
    }
    
    return func(pars);
}

static struct ast_node* parse_paren_expr(struct parser* pars)
{
    switch (pars->current.type_paren){
        case PAR_LBRACE: return parse_block(pars);            
        case PAR_LBRACKET: return parse_array(pars);            
        case PAR_LPAREN: {
            advance_token(pars);
            
            struct ast_node* node = parse_expr(pars);
            if(!node) return NULL;
            
            if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
                free_ast(node);
                return NULL;
            }
            return node;
        }
        default: return NULL;
    }
}

static struct ast_node* parse_literal_expr(struct parser* pars)
{
    if(pars->current.type_literal == LIT_IDENT){
        if(pars->peek.category == CATEGORY_OPERATOR && pars->peek.type_operator == OPER_ASSIGN){
            return parse_var_decl(pars);
        }
        if(pars->peek.category == CATEGORY_PAREN && pars->peek.type_paren == PAR_LPAREN){
            return parse_func_call(pars);
        }        
        return parse_var_ref(pars);
    }
    
    struct ast_node* node = new_ast(NODE_LITERAL);
    if(!node) return NULL;

    node->literal.value = pars->current.literal ? strdup(pars->current.literal) : strdup("");
    node->literal.type = pars->current.type_literal;
    
    advance_token(pars);
    return node;
}

static struct ast_node* parse_operator_expr(struct parser* pars)
{
    bool is_unary = (pars->peek.category == CATEGORY_LITERAL) ||
                    (pars->peek.category == CATEGORY_PAREN &&
                    (pars->peek.type_paren == PAR_LPAREN ||
                     pars->peek.type_paren == PAR_LBRACE ||
                     pars->peek.type_paren == PAR_LBRACKET));
    
    return is_unary ? parse_unary_op(pars) : parse_bin_op(pars, 0);
}

static struct ast_node* parse_keyword_stmt(struct parser* pars)
{
    int kw = pars->current.type_keyword;

    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];
    if(!func){
        fprintf(stderr, "No parser for keyword statement: %d\n", kw);
        return NULL;
    }
    
    return func(pars);
}

struct ast_node* parse_stmt(struct parser* pars)
{
    if(!pars) return NULL;
    
    if(pars->current.category == CATEGORY_KEYWORD){
        return parse_keyword_stmt(pars);
    }
    
    if(pars->current.category == CATEGORY_PAREN && 
        pars->current.type_paren == PAR_LBRACE){
        return parse_block(pars);
    }
    
    return parse_expr(pars);
}

static bool add_block_stmt(struct ast_node* block, struct ast_node* stmt)
{
    if(block->type != NODE_BLOCK) return false;
    
    if(block->block.count >= block->block.capacity){
        size_t new_capacity = block->block.capacity == 0 ? 4 : block->block.capacity * 2;
        struct ast_node** new_statements = realloc(block->block.statements, 
                                                 new_capacity * sizeof(struct ast_node*));
        if(!new_statements) return false;
        
        block->block.statements = new_statements;
        block->block.capacity = new_capacity;
    }
    
    block->block.statements[block->block.count++] = stmt;
    return true;
}

static struct ast_node* parse_block(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_BLOCK);
    if(!node) return NULL;
    
    node->block.statements = NULL;
    node->block.count = 0;
    node->block.capacity = 0;
    
    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }
    
    // expect '}'
    while (pars->current.category != CATEGORY_PAREN && pars->current.type_paren != PAR_RBRACE){       
        struct ast_node* stmt = parse_stmt(pars);
        if(!stmt){
            goto error_cleanup;
        }
        
        if(!add_block_stmt(node, stmt)){
            free_ast(stmt);
            goto error_cleanup;
        }
    }
    
    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, PARSER_ERROR_EXPECTED_PAREN)){
        goto error_cleanup;
    }

    return node;

error_cleanup:
    for (size_t i = 0; i < node->block.count; i++){
        free_ast(node->block.statements[i]);
    }
    free(node->block.statements);
    free_ast(node);
    return NULL;
}

static struct ast_node* parse_func_call(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC_CALL);
    if(!node) return NULL;

    // expect function name
    if(pars->current.category != CATEGORY_LITERAL || pars->current.type_literal != LIT_IDENT){
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                               pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    
    node->func_call.name = strdup(pars->current.literal);
    advance_token(pars);

    // expect '('
    if(consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }

    // parse arguments (comma separated)
    node->func_call.args = NULL;
    node->func_call.arg_count = 0;
    size_t cap = 0;

    if(!(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_RPAREN)){
        while (1){
            struct ast_node* arg = parse_expr(pars);
            if(!arg){
                // cleanup
                for (size_t i = 0; i < node->func_call.arg_count; ++i) free_ast(node->func_call.args[i]);
                free(node->func_call.args);
                free_ast(node);
                return NULL;
            }

            if(node->func_call.arg_count >= cap){
                size_t new_cap = cap == 0 ? 4 : cap * 2;
                struct ast_node** new_arr = (struct ast_node**)realloc(node->func_call.args, new_cap * sizeof(struct ast_node*));
                if(!new_arr){
                    free_ast(arg);
                    for (size_t i = 0; i < node->func_call.arg_count; ++i) free_ast(node->func_call.args[i]);
                    free(node->func_call.args);
                    free_ast(node);
                    return NULL;
                }
                node->func_call.args = new_arr;
                cap = new_cap;
            }
            node->func_call.args[node->func_call.arg_count++] = arg;

            if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA){
                advance_token(pars); // consume comma
                continue;
            }
            break;
        }
    }

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        for (size_t i = 0; i < node->func_call.arg_count; ++i) free_ast(node->func_call.args[i]);
        free(node->func_call.args);
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_var_decl(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR);
    if(!node) return NULL;
    
    node->var_decl = (struct node_var*)malloc(sizeof(struct node_var));
    if(!node->var_decl){free_ast(node); return NULL;}

    if(pars->current.category != CATEGORY_MODIFIER){
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_TYPE,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }    
    node->var_decl->modif= pars->current.type_modifier;
    advance_token(pars);
    
    if(pars->current.category != CATEGORY_LITERAL || pars->current.type_literal != LIT_IDENT){
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->var_decl->name = strdup(pars->current.literal);
    advance_token(pars);

    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COLON){
        advance_token(pars);
        if(pars->current.category != CATEGORY_DATATYPE){
            struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_TYPE,
                                   pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
        node->var_decl->dtype = pars->current.type_datatype;
        advance_token(pars);
    }

    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_ASSIGN){
        advance_token(pars);
        node->var_decl->value = parse_expr(pars);
        if(!node->var_decl->value){
            struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_EXPRESSION,
                                   pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
    }

    return node;
}

static struct ast_node* parse_var_ref(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR_REF);
    if (!node) return NULL;

    if (pars->current.category != CATEGORY_LITERAL || 
        pars->current.type_literal != LIT_IDENT) {
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }

    node->var_ref.name = strdup(pars->current.literal);
    if (!node->var_ref.name) {
        free_ast(node);
        return NULL;
    }

    advance_token(pars);

    return node;
}

static struct ast_node* parse_return(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_RETURN);
    if(!node) return NULL;

    node->return_stmt.body = parse_expr(pars);
    if(!node->return_stmt.body){free_ast(node); return NULL;}

    return node;
}

static struct ast_node* parse_primary(struct parser* pars);
static int get_operator_precedence(enum category_operator op);

static struct ast_node* parse_bin_op(struct parser* pars, int min_precedence){
    struct ast_node* left = parse_primary(pars);
    if(!left) return NULL;

    while (1){
        if(pars->current.category != CATEGORY_OPERATOR){
            break;
        }

        enum category_operator op = pars->current.type_operator;
        int precedence = get_operator_precedence(op);
        
        if(precedence < min_precedence){
            break;
        }

        advance_token(pars);

        struct ast_node* right = parse_bin_op(pars, precedence + 1);
        if(!right){
            free_ast(left);
            return NULL;
        }

        struct ast_node* bin_op = new_ast(NODE_BIN_OP);
        if(!bin_op){
            free_ast(left);
            free_ast(right);
            return NULL;
        }

        bin_op->bin_op.left = left;
        bin_op->bin_op.right = right;
        bin_op->bin_op.code = op;

        left = bin_op;
    }

    return left;
}

static struct ast_node* parse_primary(struct parser* pars){
    switch (pars->current.category){
        case CATEGORY_LITERAL:
            if(pars->current.type_literal == LIT_IDENT){
                return parse_var_ref(pars);
            }
            else{
                struct ast_node* node = new_ast(NODE_LITERAL);
                if(!node) return NULL;
                node->literal.value = strdup(pars->current.literal);
                node->literal.type = pars->current.type_literal;
                advance_token(pars);
                return node;
            }

        case CATEGORY_PAREN:
            if(pars->current.type_paren == PAR_LPAREN){
                advance_token(pars);
                struct ast_node* expr = parse_expr(pars);
                if(!expr) return NULL;
                if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
                    free_ast(expr);
                    return NULL;
                }
                return expr;
            }
            break;

        default: break;
    }
    return NULL;
}

static int get_operator_precedence(enum category_operator op){
    switch (op){
        case OPER_MUL: case OPER_DIV: case OPER_MOD:
            return 7;
        case OPER_ADD: case OPER_SUB:
            return 6;
        case OPER_LANGLE: case OPER_RANGE: case OPER_LTE: case OPER_GTE:
            return 5;
        case OPER_EQ: case OPER_NEQ:
            return 4;
        case OPER_AND:
            return 3;
        case OPER_OR:
            return 2;
        case OPER_ASSIGN:
            return 1;
        default:
            return 0;
    }
}

static struct ast_node* parse_unary_op(struct parser* pars)
{
    
}

static struct ast_node* parse_array(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ARRAY);
    if(!node) return NULL;

    return node;
}

static struct ast_node* parse_if(struct parser* pars){
    struct ast_node* node = new_ast(NODE_IF);
    if(!node) return NULL;

    node->if_stmt = (struct node_if*)malloc(sizeof(struct node_if));
    if(!node->if_stmt){
        free_ast(node);
        return NULL;
    }

    if(!consume_token(pars, CATEGORY_KEYWORD, KW_IF, PARSER_ERROR_EXPECTED_KEYWORD)){
        free_ast(node);
        return NULL;
    }

    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        fprintf(stderr, "Expected '(' after 'if'\n");
        free_ast(node);
        return NULL;
    }

    node->if_stmt->condition = parse_expr(pars);
    if(!node->if_stmt->condition){
        fprintf(stderr, "Failed to parse if condition\n");
        free_ast(node);
        return NULL;
    }

    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        fprintf(stderr, "Expected ')' after ifcondition\n");
        free_ast(node);
        return NULL;
    }

    if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
        node->if_stmt->then_block = parse_block(pars);
    } else {
        node->if_stmt->then_block = parse_stmt(pars);
    }

    if(!node->if_stmt->then_block){
        fprintf(stderr, "Failed to parse ifbody\n");
        free_ast(node);
        return NULL;
    }

    while (pars->current.category == CATEGORY_KEYWORD && 
           (pars->current.type_keyword == KW_ELIF || pars->current.type_keyword == KW_ELSE)){
        
        if(pars->current.type_keyword == KW_ELIF){
            advance_token(pars);

            if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, PARSER_ERROR_EXPECTED_PAREN)){
                fprintf(stderr, "Expected '(' after 'elif'\n");
                free_ast(node);
                return NULL;
            }

            struct ast_node* elif_condition = parse_expr(pars);
            if(!elif_condition){
                fprintf(stderr, "Failed to parse elifcondition\n");
                free_ast(node);
                return NULL;
            }

            if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
                fprintf(stderr, "Expected ')' after elifcondition\n");
                free_ast(elif_condition);
                free_ast(node);
                return NULL;
            }

            struct ast_node* elif_body;
            if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
                elif_body = parse_block(pars);
            } else {
                elif_body = parse_stmt(pars);
            }

            if(!elif_body){
                fprintf(stderr, "Failed to parse elifbody\n");
                free_ast(elif_condition);
                free_ast(node);
                return NULL;
            }

            struct ast_node* elif_node = new_ast(NODE_IF);
            if(!elif_node){
                free_ast(elif_condition);
                free_ast(elif_body);
                free_ast(node);
                return NULL;
            }

            elif_node->if_stmt = malloc(sizeof(struct node_if));
            if(!elif_node->if_stmt){
                free_ast(elif_condition);
                free_ast(elif_body);
                free_ast(elif_node);
                free_ast(node);
                return NULL;
            }

            elif_node->if_stmt->condition = elif_condition;
            elif_node->if_stmt->then_block = elif_body;
            elif_node->if_stmt->elif_blocks = NULL;
            elif_node->if_stmt->else_block = NULL;

            if(!node->if_stmt->elif_blocks){
                node->if_stmt->elif_blocks = elif_node;
            } else {
                struct ast_node* current = node->if_stmt->elif_blocks;
                while (current->if_stmt->elif_blocks){
                    current = current->if_stmt->elif_blocks;
                }
                current->if_stmt->elif_blocks = elif_node;
            }

        } else if(pars->current.type_keyword == KW_ELSE){
            advance_token(pars);

            if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
                node->if_stmt->else_block = parse_block(pars);
            } else {
                node->if_stmt->else_block = parse_stmt(pars);
            }

            if(!node->if_stmt->else_block){
                fprintf(stderr, "Failed to parse else block\n");
                free_ast(node);
                return NULL;
            }
            break; 
        }
    }
    return node;
}

static struct ast_node* parse_while(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_WHILE);
    if(!node) return NULL;

    node->while_loop = (struct node_while*)malloc(sizeof(struct node_while));
    if(!node->while_loop){
        free_ast(node);
        return NULL;
    }

    node->while_loop->condition = NULL;
    node->while_loop->body = NULL;

    if(!consume_token(pars, CATEGORY_KEYWORD, KW_WHILE, PARSER_ERROR_EXPECTED_KEYWORD)){
        free_ast(node);
        return NULL;
    }

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }

    node->while_loop->condition = parse_expr(pars);

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }

    node->while_loop->body = parse_block(pars);

    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, PARSER_ERROR_EXPECTED_PAREN)){
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_for(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FOR);
    if(!node) return NULL;
    
    return node; 
}

static struct ast_node* parse_func(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC);
    if (!node) return NULL;

    node->func_decl = malloc(sizeof(struct node_func));
    if (!node->func_decl) {
        free_ast(node);
        return NULL;
    }

    node->func_decl->name = NULL;
    node->func_decl->params = NULL;
    node->func_decl->param_count = 0;
    node->func_decl->return_type = DT_VOID;
    node->func_decl->body = NULL;

    // expect 'func'
    if (!consume_token(pars, CATEGORY_KEYWORD, KW_FUNC, PARSER_ERROR_EXPECTED_KEYWORD)) {
        free_ast(node);
        return NULL;
    }

    // expect function name
    if (pars->current.category != CATEGORY_LITERAL || pars->current.type_literal != LIT_IDENT) {
        struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_NAME,
                                pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    
    node->func_decl->name = strdup(pars->current.literal);
    if (!node->func_decl->name) {
        free_ast(node);
        return NULL;
    }
    advance_token(pars);

    // expect '('
    if (!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, PARSER_ERROR_EXPECTED_PAREN)) {
        free_ast(node);
        return NULL;
    }

    size_t params_capacity = 4;
    node->func_decl->params = malloc(params_capacity * sizeof(struct ast_node*));
    if (!node->func_decl->params) {
        free_ast(node);
        return NULL;
    }

    // parsing params until ')'
    while (!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, PARSER_ERROR_EXPECTED_PAREN)) {
        struct ast_node* param = parse_var_decl(pars);
        if (!param) {
            free(node->func_decl->params);
            free_ast(node);
            return NULL;
        }

        if (param->type != NODE_VAR) {
            struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_PARAM,
                                    pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
            new_parser_error(pars, err);
            free_ast(param);
            free(node->func_decl->params);
            free_ast(node);
            return NULL;
        }

        if (node->func_decl->param_count >= params_capacity) {
            params_capacity *= 2;
            struct ast_node** new_params = realloc(node->func_decl->params, 
                                                 params_capacity * sizeof(struct ast_node*));
            if (!new_params) {
                free_ast(param);
                free(node->func_decl->params);
                free_ast(node);
                return NULL;
            }
            node->func_decl->params = new_params;
        }

        node->func_decl->params[node->func_decl->param_count++] = param;

        if (pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA) {
            advance_token(pars);
        }
    }

    if (pars->current.category == CATEGORY_OPERATOR && 
        pars->current.type_operator == OPER_ARROW) {
        advance_token(pars);
        
        if (pars->current.category != CATEGORY_DATATYPE) {
            struct error* err = new_error(SEVERITY_ERROR, ERROR_TYPE_PARSER, PARSER_ERROR_EXPECTED_TYPE,
                                    pars->lexer->line, pars->lexer->column, 1, pars->lexer->input);
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
        
        node->func_decl->return_type = pars->current.type_datatype;
        advance_token(pars);
    }

    node->func_decl->body = parse_block(pars);
    if (!node->func_decl->body) {
        free_ast(node);
        return NULL;
    }

    return node;
}

static struct ast_node* parse_struct(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_STRUCT);
    if(!node) return NULL;

    return node;
}

static struct ast_node* parse_union(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_UNION);
    if(!node) return NULL;

    return node;
}

static struct ast_node* parse_enum(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ENUM);
    if(!node) return NULL;

    return node;
}

static struct ast_node* parse_match(struct parser* pars)
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

    return node; 
}

static struct ast_node* parse_import(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_IMPORT);
    if(!node) return NULL;

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
