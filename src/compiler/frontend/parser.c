#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler/frontend/parser.h"
#include "common/utils.h"

struct ast_node* parse_expr(struct parser* pars);
struct ast_node* parse_operator_expr(struct parser* pars);
struct ast_node* parse_keyword_expr(struct parser* pars);
struct ast_node* parse_paren_expr(struct parser* pars);
struct ast_node* parse_literal_expr(struct parser* pars);

struct ast_node* parse_block(struct parser* pars);
struct ast_node* parse_postfix(struct parser* pars);
struct ast_node* parse_unary_op(struct parser* pars);
struct ast_node* parse_bin_op(struct parser* pars, int min_precedence);

struct ast_node* parse_func_call(struct parser* pars);
struct ast_node* parse_var_decl(struct parser* pars);
struct ast_node* parse_var_ref(struct parser* pars);
struct ast_node* parse_return(struct parser* pars);
struct ast_node* parse_break(struct parser* pars);
struct ast_node* parse_continue(struct parser* pars);
struct ast_node* parse_if(struct parser* pars);
struct ast_node* parse_while(struct parser* pars);
struct ast_node* parse_for(struct parser* pars);
struct ast_node* parse_func(struct parser* pars);
struct ast_node* parse_array(struct parser* pars);
struct ast_node* parse_struct(struct parser* pars);
struct ast_node* parse_union(struct parser* pars);
struct ast_node* parse_enum(struct parser* pars);
struct ast_node* parse_match(struct parser* pars);
struct ast_node* parse_trait(struct parser* pars);
struct ast_node* parse_try_catch(struct parser* pars);
struct ast_node* parse_import(struct parser* pars);
// struct ast_node* parse_test(struct parser* pars);
// struct ast_node* parse_fork(struct parser* pars);
// struct ast_node* parse_solve(struct parser* pars);
// struct ast_node* parse_simulate(struct parser* pars);

typedef struct ast_node* (*parse_func_t)(struct parser*);

parse_func_t parse_table[] = {
    [KW_IF]       = parse_if,
    [KW_WHILE]    = parse_while,
    [KW_FOR]      = parse_for,
    [KW_FUNC]     = parse_func,
    [KW_STRUCT]   = parse_struct,
    [KW_ENUM]     = parse_enum,
    [KW_UNION]    = parse_union,
    [KW_MATCH]    = parse_match,
    [KW_IMPL]     = parse_trait,
    [KW_TRY]      = parse_try_catch,
    [KW_IMPORT]   = parse_import,
    [KW_RETURN]   = parse_return,
    [KW_BREAK]    = parse_break,
    [KW_CONTINUE] = parse_continue,
    // [KW_TEST]     = parse_test,
    // [KW_FORK]     = parse_fork,
    // [KW_SOLVE]    = parse_solve,
    // [KW_SIMULATE] = parse_simulate,
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

    return parser;
}

void new_parser_error(struct parser* pars, struct report* err)
{
    if(!pars || !err){
        if(err) free_report(err);
        return;
    }

    if(!pars->errors){
        pars->errors = (struct report**)malloc(sizeof(struct report*));
        if(!pars->errors){
            free_report(err);
            return;
        }
        pars->errors[0] = err;
        pars->errors_count = 1;
        return;
    }

    struct report** new_errors = (struct report**)realloc(pars->errors, (pars->errors_count + 1) * sizeof(struct report*));
    if(!new_errors){
        free_report(err);
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
            if(parser->errors[i]) free_report(parser->errors[i]);
        }
        free(parser->errors);
        parser->errors = NULL;
        parser->errors_count = 0;
    }
    if(parser->lexer) free_lexer(parser->lexer);
    free(parser);
}

void advance_token(struct parser *pars)
{
    if(!pars || (pars->current.category == CATEGORY_SERVICE && pars->current.type_service == SERV_EOF)) return;

    free_token(&pars->current);
    
    pars->current = pars->peek;
    pars->peek = next_token(pars->lexer);
}

bool consume_token(struct parser *pars, const enum category_tag expected_category, const int expected_type, const enum report_code err)
{
    if(!pars) return false;
    
    if(pars->current.category != expected_category){
        struct report* new_err = new_report(
            SEVERITY_ERROR, err, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, new_err);
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
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_UNEXPECTED_TOKEN, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            return false;
        }
    }

    advance_token(pars);
    return true;
}

struct ast_node* parse_program(struct parser* pars)
{
    struct ast_node* ast = NULL;

    while(pars->current.category != CATEGORY_SERVICE || pars->current.type_service != SERV_EOF) {
        ast = parse_expr(pars);
        if(!ast) return NULL;
    }

    return ast;
}

struct ast_node* parse_expr(struct parser* pars)
{
    if(!pars) return NULL;

    switch(pars->current.category){
        case CATEGORY_OPERATOR: return parse_operator_expr(pars);
        case CATEGORY_KEYWORD:  return parse_keyword_expr(pars);
        case CATEGORY_PAREN:    return parse_paren_expr(pars);
        case CATEGORY_MODIFIER: return parse_var_decl(pars);
        case CATEGORY_LITERAL:  return parse_bin_op(pars, 0);
        default: return NULL;
    }

    return NULL;
}

struct ast_node* parse_keyword_expr(struct parser* pars)
{
    const int kw = pars->current.type_keyword;
    
    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];

    if(!func){
        return NULL;
    }
    
    return func(pars);
}

struct ast_node* parse_paren_expr(struct parser* pars)
{
    switch(pars->current.type_paren){
        case PAR_LBRACE:   return parse_block(pars);            
        case PAR_LBRACKET: return parse_array(pars);            
        case PAR_LPAREN:
            advance_token(pars);
            
            struct ast_node* node = parse_expr(pars);
            if(!node) return NULL;
            
            if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){
                free_ast(node);
                return NULL;
            }
            return node;
        default: return NULL;
    }
}

struct ast_node* parse_literal_expr(struct parser* pars)
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

    node->literal.value = pars->current.literal ? util_strdup(pars->current.literal) : util_strdup("");
    node->literal.type = pars->current.type_literal;
    
    advance_token(pars);
    return node;
}

struct ast_node* parse_operator_expr(struct parser* pars)
{
    bool is_unary = (pars->peek.category == CATEGORY_LITERAL) ||
                    (pars->peek.category == CATEGORY_PAREN &&
                    (pars->peek.type_paren == PAR_LPAREN ||
                     pars->peek.type_paren == PAR_LBRACE ||
                     pars->peek.type_paren == PAR_LBRACKET));
    
    return is_unary ? parse_unary_op(pars) : parse_bin_op(pars, 0);
}

struct ast_node* parse_keyword_stmt(struct parser* pars)
{
    int kw = pars->current.type_keyword;

    if(kw < 0 || (size_t)kw >= sizeof(parse_table)/sizeof(parse_table[0])){
        return NULL;
    }
    
    parse_func_t func = parse_table[kw];
    if(!func){
        return NULL;
    }
    
    return func(pars);
}

struct ast_node* parse_stmt(struct parser* pars)
{
    if(!pars) return NULL;
    
    // skip empty statements
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_SEMICOLON){
        advance_token(pars);
        return NULL;
    }
    
    if(pars->current.category == CATEGORY_KEYWORD){
        return parse_keyword_stmt(pars);
    }
    
    if(pars->current.category == CATEGORY_PAREN && 
        pars->current.type_paren == PAR_LBRACE){
        return parse_block(pars);
    }
    
    return parse_expr(pars);
}

bool add_block_stmt(struct ast_node* block, struct ast_node* stmt)
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

struct ast_node* parse_block(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_BLOCK);
    if(!node) return NULL;
    
    node->block.statements = NULL;
    node->block.count = 0;
    node->block.capacity = 0;
        
    advance_token(pars); // skip '{'
    
    // parse statements until '}'
    while(pars->current.category != CATEGORY_PAREN && pars->current.type_paren != PAR_RBRACE){

        // check for EOF
        if(pars->current.category == CATEGORY_SERVICE && pars->current.type_service == SERV_EOF){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_PAREN, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            goto error_cleanup;
        }
        
        // parse statement
        struct ast_node* stmt = parse_stmt(pars);
        if(!stmt) continue;
        
        // add to block
        if(!add_block_stmt(node, stmt)){
            free_ast(stmt);
            goto error_cleanup;
        }
        
        // optionally consume ';'
        if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_SEMICOLON) {
            advance_token(pars);
        }
    }
    
    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){
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

struct ast_node* parse_func_call(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC_CALL);
    if(!node) return NULL;

    // expect function name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    
    node->func_call.name = util_strdup(pars->current.literal);
    advance_token(pars);

    advance_token(pars); // expect '('

    // parse arguments (comma separated)
    node->func_call.args = NULL;
    node->func_call.arg_count = 0;
    size_t cap = 0;

    if(!(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_RPAREN)){
        while(1){
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

            // consume ','
            if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA){ advance_token(pars); continue; }
            break;
        }
    }

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){
        for (size_t i = 0; i < node->func_call.arg_count; ++i) free_ast(node->func_call.args[i]);
        free(node->func_call.args);
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_var_decl(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR);
    if(!node) return NULL;
    
    node->var_decl = (struct node_var*)malloc(sizeof(struct node_var));
    if(!node->var_decl){
        free_ast(node);
        return NULL;
    }

    node->var_decl->modif = MOD_VAR;
    node->var_decl->name = NULL;
    node->var_decl->dtype = DT_INT;
    node->var_decl->value = NULL;

    // optional modifier
    if(pars->current.category == CATEGORY_MODIFIER){
        node->var_decl->modif = pars->current.type_modifier;
        advance_token(pars);
    }
    
    // expect identifier
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->var_decl->name = util_strdup(pars->current.literal);
    if(!node->var_decl->name){
        free_ast(node);
        return NULL;
    }
    advance_token(pars);

    // optional type annotation
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COLON){
        advance_token(pars);
        if(pars->current.category != CATEGORY_DATATYPE){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_TYPE, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
        node->var_decl->dtype = pars->current.type_datatype;
        advance_token(pars);
    }

    // optional assignment
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_ASSIGN){
        advance_token(pars);
        node->var_decl->value = parse_expr(pars);
        if(!node->var_decl->value){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_EXPRESSION, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
    }

    return node;
}

struct ast_node* parse_var_ref(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_VAR_REF);
    if(!node) return NULL;

    node->var_ref.name = NULL;
    
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }

    node->var_ref.name = util_strdup(pars->current.literal);
    if(!node->var_ref.name){
        free_ast(node);
        return NULL;
    }

    advance_token(pars);

    return node;
}

struct ast_node* parse_return(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_RETURN);
    if(!node) return NULL;

    advance_token(pars); // skip 'return'

    node->return_stmt.body = parse_expr(pars);
    if(!node->return_stmt.body){free_ast(node); return NULL;}

    // optionally consume ';'
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_SEMICOLON){
        advance_token(pars);
    }

    return node;
}

struct ast_node* parse_break(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_BREAK);
    if(!node) return NULL;

    advance_token(pars); // skip 'break'

    // optionally consume ';'
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_SEMICOLON){
        advance_token(pars);
    }

    return node;
}
struct ast_node* parse_continue(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_CONTINUE);
    if(!node) return NULL;

    // skip 'continue'
    advance_token(pars);

    // optionally consume ';'
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_SEMICOLON){
        advance_token(pars);
    }

    return node;
}

struct ast_node* parse_primary(struct parser* pars)
{
    switch (pars->current.category){
        case CATEGORY_LITERAL:
            if(pars->current.type_literal == LIT_IDENT){
                if(pars->peek.category == CATEGORY_PAREN && pars->peek.type_paren == PAR_LPAREN){
                    return parse_func_call(pars);
                }
                return parse_var_ref(pars);
            }
            else{
                struct ast_node* node = new_ast(NODE_LITERAL);
                if(!node) return NULL;
                node->literal.value = util_strdup(pars->current.literal);
                node->literal.type = pars->current.type_literal;
                advance_token(pars);
                return node;
            }

        case CATEGORY_PAREN:
            if(pars->current.type_paren == PAR_LPAREN){
                advance_token(pars);
                struct ast_node* expr = parse_expr(pars);
                if(!expr) return NULL;
                if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){
                    free_ast(expr);
                    return NULL;
                }
                return expr;
            }
            else if(pars->current.type_paren == PAR_LBRACKET){
                return parse_array(pars);
            }
            break;
        case CATEGORY_OPERATOR:
            // check for unary operators
            if(pars->current.type_operator == OPER_PLUS || pars->current.type_operator == OPER_MINUS || pars->current.type_operator == OPER_NOT){
                return parse_unary_op(pars);
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
    switch (op){
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

// enum op_code operator_to_opcode(enum category_operator op)
// {
//     switch(op){
//         case OPER_PLUS: case OPER_ADD:
//             return OP_ADD;

//         case OPER_MINUS: case OPER_SUB:
//             return OP_SUB;

//         case OPER_ASTERISK: case OPER_MUL:
//             return OP_MUL;

//         case OPER_SLASH: case OPER_DIV:
//             return OP_DIV;

//         case OPER_INCREM: case OPER_DECREM:
//             return OP_DUP;

//         case OPER_EQ:     return OP_EQ;
//         case OPER_NEQ:    return OP_NEQ;
//         case OPER_LANGLE: return OP_LT;
//         case OPER_RANGLE: return OP_GT;
//         case OPER_AND:    return OP_AND;
//         case OPER_OR:     return OP_OR;
//         case OPER_NOT:    return OP_NOT;
//         case OPER_ASSIGN: return OP_STORE;
        
//         default: return OP_PUSH;
//     }
// }

struct ast_node* parse_postfix(struct parser* pars)
{
    struct ast_node* expr = parse_primary(pars);
    if(!expr) return NULL;

    while(pars->current.category == CATEGORY_OPERATOR){
        enum category_operator op = pars->current.type_operator;
        
        if(op == OPER_INCREM || op == OPER_DECREM){
            struct ast_node* postfix = new_ast(NODE_UNARY_OP);
            if(!postfix){
                free_ast(expr);
                return NULL;
            }
            
            postfix->unary_op.right = expr;
            // postfix->unary_op.code = operator_to_opcode(op);
            postfix->unary_op.is_postfix = true;
            
            advance_token(pars);
            expr = postfix;
        }
        else break;
    }
    return expr;
}

struct ast_node* parse_bin_op(struct parser* pars, int min_precedence)
{
    struct ast_node* left = parse_postfix(pars);
    if(!left) return NULL;

    while(pars->current.category == CATEGORY_OPERATOR){
        enum category_operator op = pars->current.type_operator;
        
        if(op == OPER_SEMICOLON || op == OPER_COMMA) break;
        
        int precedence = get_operator_precedence(op);
        
        if(precedence == 0) break;
        
        if(precedence < min_precedence) break;

        advance_token(pars);

        // calculate next minimum precedence
        int next_min_prec;
        if (is_right_associative(op)) next_min_prec = precedence;
        else next_min_prec = precedence + 1;

        // parse right operand
        struct ast_node* right = parse_bin_op(pars, next_min_prec);
        if(!right){
            free_ast(left);
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_EXPRESSION, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
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
        // bin_op->bin_op.code = operator_to_opcode(op);

        left = bin_op;
    }

    return left;
}

struct ast_node* parse_unary_op(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_UNARY_OP);
    if(!node) return NULL;

    enum category_operator op_type = pars->current.type_operator;
    
    if(op_type != OPER_PLUS && op_type != OPER_MINUS && op_type != OPER_NOT && op_type != OPER_INCREM && op_type != OPER_DECREM){
        free_ast(node);
        return NULL;
    }

    // node->unary_op.code = operator_to_opcode(op_type);
    node->unary_op.is_postfix = false;
    advance_token(pars);

    if(op_type == OPER_INCREM || op_type == OPER_DECREM){
        node->unary_op.right = parse_postfix(pars);
    }
    else{
        node->unary_op.right = parse_primary(pars);
    }
    
    if(!node->unary_op.right){
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_array(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ARRAY);
    if(!node) return NULL;

    node->array_decl = (struct node_array*)malloc(sizeof(struct node_array));
    if(!node->array_decl){
        free_ast(node);
        return NULL;
    }

    node->array_decl->elements = NULL;
    node->array_decl->count = 0;
    node->array_decl->capacity = 0;

    advance_token(pars); // skip '['

    // parse elements (comma separated)
    if(!(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_RBRACKET)){
        while(1){
            struct ast_node* element = parse_primary(pars);
            if(!element){
                // cleanup
                for (size_t i = 0; i < node->array_decl->count; ++i) free_ast(node->array_decl->elements[i]);
                free(node->array_decl->elements);
                free_ast(node);
                return NULL;
            }

            if(node->array_decl->count >= node->array_decl->capacity){
                size_t new_cap = node->array_decl->capacity == 0 ? 4 : node->array_decl->capacity * 2;
                struct ast_node** new_arr = (struct ast_node**)realloc(node->array_decl->elements, new_cap * sizeof(struct ast_node*));
                if(!new_arr){
                    free_ast(element);
                    for (size_t i = 0; i < node->array_decl->count; ++i) free_ast(node->array_decl->elements[i]);
                    free(node->array_decl->elements);
                    free_ast(node);
                    return NULL;
                }
                node->array_decl->elements = new_arr;
                node->array_decl->capacity = new_cap;
            }
            node->array_decl->elements[node->array_decl->count++] = element;

            if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA){
                advance_token(pars); // consume comma
                continue;
            }
            break;
        }
    }

    // expect ']'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACKET, ERROR_EXPECTED_PAREN)){
        for (size_t i = 0; i < node->array_decl->count; ++i) free_ast(node->array_decl->elements[i]);
        free(node->array_decl->elements);
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_if(struct parser* pars){
    struct ast_node* node = new_ast(NODE_IF);
    if(!node) return NULL;

    node->if_stmt = (struct node_if*)malloc(sizeof(struct node_if));
    if(!node->if_stmt){
        free_ast(node);
        return NULL;
    }
    
    node->if_stmt->condition = NULL;
    node->if_stmt->then_block = NULL;
    node->if_stmt->else_block = NULL;
    node->if_stmt->elif_blocks = NULL;

    advance_token(pars); // skip 'if'

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    node->if_stmt->condition = parse_expr(pars);
    if(!node->if_stmt->condition){
        free_ast(node);
        return NULL;
    }

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

    // expect '{'
    if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
        node->if_stmt->then_block = parse_block(pars);
    }
    else{
        node->if_stmt->then_block = parse_stmt(pars);
    }

    if(!node->if_stmt->then_block){
        free_ast(node);
        return NULL;
    }

    // parse elif/else blocks
    while(pars->current.category == CATEGORY_KEYWORD && (pars->current.type_keyword == KW_ELIF || pars->current.type_keyword == KW_ELSE)){

        // expect optional 'elif'
        if(pars->current.type_keyword == KW_ELIF){
            advance_token(pars);

            // expect '('
            if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

            struct ast_node* elif_condition = parse_expr(pars);
            if(!elif_condition){
                free_ast(node);
                return NULL;
            }

            // expect ')'
            if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){ free_ast(elif_condition); free_ast(node); return NULL; }

            struct ast_node* elif_body;

            // expect '{'
            if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
                elif_body = parse_block(pars);
            }
            else{
                elif_body = parse_stmt(pars);
            }

            if(!elif_body){
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
            }
            else{
                struct ast_node* current = node->if_stmt->elif_blocks;
                while(current->if_stmt->elif_blocks){
                    current = current->if_stmt->elif_blocks;
                }
                current->if_stmt->elif_blocks = elif_node;
            }
        }

        // expect optional 'else'
        else if(pars->current.type_keyword == KW_ELSE){
            advance_token(pars);

            // expect '{'
            if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
                node->if_stmt->else_block = parse_block(pars);
            }
            else{
                node->if_stmt->else_block = parse_stmt(pars);
            }

            if(!node->if_stmt->else_block){
                free_ast(node);
                return NULL;
            }
            break; 
        }
    }
    return node;
}

struct ast_node* parse_while(struct parser* pars)
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

    advance_token(pars); // skip 'while'

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    node->while_loop->condition = parse_expr(pars);
    if(!node->while_loop->condition){
        free_ast(node);
        return NULL;
    }

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

    // parse body (can be a block or a single statement)
    if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
        node->while_loop->body = parse_block(pars);
    }
    else{
        node->while_loop->body = parse_stmt(pars);
    }
    
    if(!node->while_loop->body){
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_for(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FOR);
    if(!node) return NULL;
    
    node->for_loop = (struct node_for*)malloc(sizeof(struct node_for));
    if(!node->for_loop){
        free_ast(node);
        return NULL;
    }

    node->for_loop->init = NULL;
    node->for_loop->condition = NULL;
    node->for_loop->update = NULL;
    node->for_loop->body = NULL;

    advance_token(pars); // skip 'for'

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse init statement
    if(pars->current.category != CATEGORY_OPERATOR && pars->current.type_operator != OPER_SEMICOLON){
        node->for_loop->init = parse_expr(pars);
        if(!node->for_loop->init){
            free_ast(node);
            return NULL;
        }
    }

    // expect ';'
    if(!consume_token(pars, CATEGORY_OPERATOR, OPER_SEMICOLON, ERROR_EXPECTED_DELIMITER)){ free_ast(node); return NULL; }

    // parse condition
    if(pars->current.category != CATEGORY_OPERATOR && pars->current.type_operator != OPER_SEMICOLON){
        node->for_loop->condition = parse_expr(pars);
        if(!node->for_loop->condition){
            free_ast(node);
            return NULL;
        }
    }

    // expect ';'
    if(!consume_token(pars, CATEGORY_OPERATOR, OPER_SEMICOLON, ERROR_EXPECTED_DELIMITER)){ free_ast(node); return NULL; }

    // parse update statement
    if(pars->current.category != CATEGORY_PAREN && pars->current.type_paren != PAR_RPAREN){
        node->for_loop->update = parse_expr(pars);
        if(!node->for_loop->update){
            free_ast(node);
            return NULL;
        }
    }

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

    // Parse body (can be a block or a single statement)
    if(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_LBRACE){
        node->for_loop->body = parse_block(pars);
    }
    else{
        node->for_loop->body = parse_stmt(pars);
    }
    
    if(!node->for_loop->body){
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_func(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_FUNC);
    if(!node) return NULL;

    node->func_decl = malloc(sizeof(struct node_func));
    if(!node->func_decl){
        free_ast(node);
        return NULL;
    }

    node->func_decl->name = NULL;
    node->func_decl->params = NULL;
    node->func_decl->param_count = 0;
    node->func_decl->return_type = DT_VOID;
    node->func_decl->body = NULL;

    advance_token(pars); // skip 'func'

    // expect function name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    
    node->func_decl->name = util_strdup(pars->current.literal);
    if(!node->func_decl->name){
        free_ast(node);
        return NULL;
    }
    advance_token(pars);

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    size_t params_capacity = 4;
    node->func_decl->params = malloc(params_capacity * sizeof(struct ast_node*));
    if(!node->func_decl->params){
        free_ast(node);
        return NULL;
    }

    // parsing params until ')'
    if(!(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_RPAREN)){
        while(1){
            struct ast_node* param = parse_var_decl(pars);
            if(!param){
                for(size_t i = 0; i < node->func_decl->param_count; i++){
                    free_ast(node->func_decl->params[i]);
                }
                free(node->func_decl->params);
                free_ast(node);
                return NULL;
            }

            // check if param is a variable
            if(param->type != NODE_VAR){
                struct report* err = new_report(
                    SEVERITY_ERROR, ERROR_EXPECTED_PARAM, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
                );
                new_parser_error(pars, err);
                free_ast(param);
                for(size_t i = 0; i < node->func_decl->param_count; i++){
                    free_ast(node->func_decl->params[i]);
                }
                free(node->func_decl->params);
                free_ast(node);
                return NULL;
            }

            // check if there is enough capacity
            if(node->func_decl->param_count >= params_capacity){
                params_capacity *= 2;
                struct ast_node** new_params = realloc(node->func_decl->params, 
                                                     params_capacity * sizeof(struct ast_node*));
                if(!new_params){
                    free_ast(param);
                    for(size_t i = 0; i < node->func_decl->param_count; i++){
                        free_ast(node->func_decl->params[i]);
                    }
                    free(node->func_decl->params);
                    free_ast(node);
                    return NULL;
                }
                node->func_decl->params = new_params;
            }

            node->func_decl->params[node->func_decl->param_count++] = param;

            // consume ','
            if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA){
                advance_token(pars);
                continue;
            }
            break;
        }
    }
    
    advance_token(pars); // consume ')'

    // optional return type
    if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COLON){
        advance_token(pars);
        
        // expect datatype
        if(pars->current.category != CATEGORY_DATATYPE){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_TYPE, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            free_ast(node);
            return NULL;
        }
        
        node->func_decl->return_type = pars->current.type_datatype;
        advance_token(pars);
    }

    // expect function body (block)
    node->func_decl->body = parse_block(pars);
    if(!node->func_decl->body){
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_struct(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_STRUCT);
    if(!node) return NULL;

    node->struct_decl = (struct node_struct*)malloc(sizeof(struct node_struct));
    if(!node->struct_decl){
        free_ast(node);
        return NULL;
    }

    node->struct_decl->name = NULL;
    node->struct_decl->members = NULL;
    node->struct_decl->member_count = 0;
    node->struct_decl->member_capacity = 0;

    advance_token(pars); // skip 'struct'

    // expect struct name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->struct_decl->name = util_strdup(pars->current.literal);
    advance_token(pars);

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse struct members
    while(pars->current.category != CATEGORY_PAREN && pars->current.type_paren != PAR_RBRACE){
        
        // check for EOF
        if(pars->current.category == CATEGORY_SERVICE && pars->current.type_service == SERV_EOF){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_PAREN, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            for(size_t i = 0; i < node->struct_decl->member_count; ++i) {
                free_ast(node->struct_decl->members[i]);
            }
            free(node->struct_decl->members);
            free_ast(node);
            return NULL;
        }
        
        // parse member variable declaration
        struct ast_node* member = parse_var_decl(pars);
        if(!member){
            // cleanup existing members
            for(size_t i = 0; i < node->struct_decl->member_count; ++i) {
                free_ast(node->struct_decl->members[i]);
            }
            free(node->struct_decl->members);
            free_ast(node);
            return NULL;
        }

        // add member
        if(node->struct_decl->member_count >= node->struct_decl->member_capacity){
            size_t new_cap = node->struct_decl->member_capacity == 0 ? 4 : node->struct_decl->member_capacity * 2;
            struct ast_node** new_members = realloc(node->struct_decl->members, new_cap * sizeof(struct ast_node*));
            if(!new_members){
                free_ast(member);
                for(size_t i = 0; i < node->struct_decl->member_count; ++i) {
                    free_ast(node->struct_decl->members[i]);
                }
                free(node->struct_decl->members);
                free_ast(node);
                return NULL;
            }
            node->struct_decl->members = new_members;
            node->struct_decl->member_capacity = new_cap;
        }
        node->struct_decl->members[node->struct_decl->member_count++] = member;
        
        // optional comma separator
        if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA) {
            advance_token(pars);
        }
    }
    
    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){
        for(size_t i = 0; i < node->struct_decl->member_count; ++i) {
            free_ast(node->struct_decl->members[i]);
        }
        free(node->struct_decl->members);
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_union(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_UNION);
    if(!node) return NULL;

    node->union_decl = (struct node_union*)malloc(sizeof(struct node_union));
    if(!node->union_decl){
        free_ast(node);
        return NULL;
    }

    node->union_decl->name = NULL;
    node->union_decl->members = NULL;
    node->union_decl->member_count = 0;
    node->union_decl->member_capacity = 0;

    // skip 'union'
    advance_token(pars);

    // expect union name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->union_decl->name = util_strdup(pars->current.literal);
    advance_token(pars);

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse union members
    while(pars->current.category != CATEGORY_PAREN || pars->current.type_paren != PAR_RBRACE){

        // check for EOF
        if(pars->current.category == CATEGORY_SERVICE && pars->current.type_service == SERV_EOF){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_PAREN, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            for(size_t i = 0; i < node->union_decl->member_count; ++i) {
                free_ast(node->union_decl->members[i]);
            }
            free(node->union_decl->members);
            free_ast(node);
            return NULL;
        }

        struct ast_node* member = parse_var_decl(pars);
        if(!member){
            // cleanup existing members
            for(size_t i = 0; i < node->union_decl->member_count; ++i) {
                free_ast(node->union_decl->members[i]);
            }
            free(node->union_decl->members);
            free_ast(node);
            return NULL;
        }

        // add member
        if(node->union_decl->member_count >= node->union_decl->member_capacity){
            size_t new_cap = node->union_decl->member_capacity == 0 ? 4 : node->union_decl->member_capacity * 2;
            struct ast_node** new_members = realloc(node->union_decl->members, new_cap * sizeof(struct ast_node*));
            if(!new_members){
                free_ast(member);
                for(size_t i = 0; i < node->union_decl->member_count; ++i) {
                    free_ast(node->union_decl->members[i]);
                }
                free(node->union_decl->members);
                free_ast(node);
                return NULL;
            }
            node->union_decl->members = new_members;
            node->union_decl->member_capacity = new_cap;
        }
        node->union_decl->members[node->union_decl->member_count++] = member;

        // optional comma separator
        if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA) {
            advance_token(pars);
        }
    }

    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){
        for(size_t i = 0; i < node->union_decl->member_count; ++i) {
            free_ast(node->union_decl->members[i]);
        }
        free(node->union_decl->members);
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_enum(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_ENUM);
    if(!node) return NULL;

    node->enum_decl = (struct node_enum*)malloc(sizeof(struct node_enum));
    if(!node->enum_decl){
        free_ast(node);
        return NULL;
    }

    node->enum_decl->name = NULL;
    node->enum_decl->members = NULL;
    node->enum_decl->member_count = 0;
    node->enum_decl->member_capacity = 0;

    advance_token(pars); // skip 'enum'

    // expect enum name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->enum_decl->name = util_strdup(pars->current.literal);
    advance_token(pars);

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse enum members
    while(pars->current.category != CATEGORY_PAREN || pars->current.type_paren != PAR_RBRACE){
        // check for EOF
        if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
            struct report* err = new_report(
                SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
            );
            new_parser_error(pars, err);
            // cleanup existing members
            for(size_t i = 0; i < node->enum_decl->member_count; ++i) {
                free(node->enum_decl->members[i]);
            }
            free(node->enum_decl->members);
            free_ast(node);
            return NULL;
        }

        // expect member name
        char* member_name = util_strdup(pars->current.literal);
        if(!member_name) {
            // cleanup existing members
            for(size_t i = 0; i < node->enum_decl->member_count; ++i) {
                free(node->enum_decl->members[i]);
            }
            free(node->enum_decl->members);
            free_ast(node);
            return NULL;
        }
        advance_token(pars);

        // add member
        if(node->enum_decl->member_count >= node->enum_decl->member_capacity){
            size_t new_cap = node->enum_decl->member_capacity == 0 ? 4 : node->enum_decl->member_capacity * 2;
            char** new_members = realloc(node->enum_decl->members, new_cap * sizeof(char*));
            if(!new_members){
                free(member_name);
                for(size_t i = 0; i < node->enum_decl->member_count; ++i) {
                    free(node->enum_decl->members[i]);
                }
                free(node->enum_decl->members);
                free_ast(node);
                return NULL;
            }
            node->enum_decl->members = new_members;
            node->enum_decl->member_capacity = new_cap;
        }
        node->enum_decl->members[node->enum_decl->member_count++] = member_name;

        // optional comma separator
        if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA) {
            advance_token(pars);
        }
    }

    return node;
}

struct ast_node* parse_match(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_MATCH);
    if(!node) return NULL;

    node->match_stmt = (struct node_match*)malloc(sizeof(struct node_match));
    if(!node->match_stmt){
        free_ast(node);
        return NULL;
    }

    node->match_stmt->target = NULL;
    node->match_stmt->cases = NULL;
    node->match_stmt->case_count = 0;
    node->match_stmt->case_capacity = 0;

    advance_token(pars); // skip 'match'

    // parse target expression
    node->match_stmt->target = parse_expr(pars);
    if(!node->match_stmt->target){
        free_ast(node);
        return NULL;
    }

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){
        free_ast(node->match_stmt->target);
        free_ast(node);
        return NULL;
    }

    // parse cases
    while(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){

        // check for EOF
        if(!consume_token(pars, CATEGORY_KEYWORD, KW_CASE, ERROR_EXPECTED_KEYWORD)){
            // Only free cases array and target, individual cases were already freed
            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target); 
            free(node->match_stmt);
            free(node);
            return NULL;
        }

        // create new case node
        struct ast_node* case_node = new_ast(NODE_CASE);
        if(!case_node){
            // Only free cases array and target, individual cases were already freed
            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target);
            free(node->match_stmt);
            free(node);
            return NULL;
        }

        // create case struct
        case_node->match_case = (struct node_case*)malloc(sizeof(struct node_case));
        if(!case_node->match_case){
            free_ast(case_node); 
            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target);
            free(node->match_stmt);
            free(node);
            return NULL;
        }

        // parse case condition 
        case_node->match_case->condition = parse_expr(pars);
        if(!case_node->match_case->condition){
            free(case_node->match_case);
            free_ast(case_node);
            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target);
            free(node->match_stmt);
            free(node);
            return NULL;
        }

        // expect '=>'
        if(!consume_token(pars, CATEGORY_OPERATOR, OPER_ARROW, ERROR_EXPECTED_OPERATOR)){
            free_ast(case_node);

            // cleanup existing cases
            for(size_t i = 0; i < node->match_stmt->case_count; ++i) {
                free_ast(node->match_stmt->cases[i]);
            }

            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target);
            free_ast(node);
            return NULL;
        }

        // parse case body (can be a block or a single statement)
        case_node->match_case->body = parse_stmt(pars);
        if(!case_node->match_case->body){
            free_ast(case_node);
            // cleanup existing cases
            for(size_t i = 0; i < node->match_stmt->case_count; ++i) {
                free_ast(node->match_stmt->cases[i]);
            }
            free(node->match_stmt->cases);
            free_ast(node->match_stmt->target);
            free_ast(node);
            return NULL;
        }

        // add case to match statement
        if(node->match_stmt->case_count >= node->match_stmt->case_capacity){
            size_t new_cap = node->match_stmt->case_capacity == 0 ? 4 : node->match_stmt->case_capacity * 2;
            struct ast_node** new_cases = realloc(node->match_stmt->cases, new_cap * sizeof(struct ast_node*));
            if(!new_cases){
                free_ast(case_node);
                // cleanup existing cases
                for(size_t i = 0; i < node->match_stmt->case_count; ++i) {
                    free_ast(node->match_stmt->cases[i]);
                }
                free(node->match_stmt->cases);
                free_ast(node->match_stmt->target);
                free_ast(node);
                return NULL;
            }
            node->match_stmt->cases = new_cases;
            node->match_stmt->case_capacity = new_cap;
        }
        node->match_stmt->cases[node->match_stmt->case_count++] = case_node;
    }

    return node;
}

struct ast_node* parse_trait(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_TRAIT);
    if(!node) return NULL;

    node->trait_decl = (struct node_trait*)malloc(sizeof(struct node_trait));
    if(!node->trait_decl){
        free_ast(node);
        return NULL;
    }

    node->trait_decl->name = NULL;
    node->trait_decl->body = NULL;

    advance_token(pars); // skip 'impl'

    // expect name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->trait_decl->name = util_strdup(pars->current.literal);
    advance_token(pars);

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse body
    node->trait_decl->body = parse_block(pars);
    if(!node->trait_decl->body){
        free_ast(node);
        return NULL;
    }

    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    return node;
}

struct ast_node* parse_try_catch(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_TRYCATCH);
    if(!node) return NULL;

    node->trycatch_stmt = (struct node_trycatch*)malloc(sizeof(struct node_trycatch));
    if(!node->trycatch_stmt){
        free_ast(node);
        return NULL;
    }

    node->trycatch_stmt->try_block = NULL;
    node->trycatch_stmt->catch_block = NULL;
    node->trycatch_stmt->finally_block = NULL;

    advance_token(pars); // skip 'try'

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse try block
    node->trycatch_stmt->try_block = parse_block(pars);
    if(!node->trycatch_stmt->try_block){
        free_ast(node);
        return NULL;
    }

    // expect '}'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

    // expect 'catch'
    if(!consume_token(pars, CATEGORY_KEYWORD, KW_CATCH, ERROR_EXPECTED_KEYWORD)){ free_ast(node); return NULL; }

    advance_token(pars);

    // expect '('
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // TODO: parse catch exception variable

    // expect ')'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL; }

    // expect '{'
    if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

    // parse catch block
    node->trycatch_stmt->catch_block = parse_block(pars);
    if(!node->trycatch_stmt->catch_block){
        free_ast(node);
        return NULL;
    }

    return node;
}

struct ast_node* parse_import(struct parser* pars)
{
    struct ast_node* node = new_ast(NODE_IMPORT);
    if(!node) return NULL;

    node->import_stmt = (struct node_import*)malloc(sizeof(struct node_import));
    if(!node->import_stmt){
        free_ast(node);
        return NULL;
    }
    node->import_stmt->module_name = NULL;

    advance_token(pars); // skip 'import'

    // expect module name
    if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
        struct report* err = new_report(
            SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
        );
        new_parser_error(pars, err);
        free_ast(node);
        return NULL;
    }
    node->import_stmt->module_name = util_strdup(pars->current.literal);
    advance_token(pars);

    return node;
}

// struct ast_node* parse_test(struct parser* pars)
// {
//     struct ast_node* node = new_ast(NODE_TEST);
//     if(!node) return NULL;

//     node->test_stmt = (struct node_test*)malloc(sizeof(struct node_test));
//     if(!node->test_stmt){
//         free_ast(node);
//         return NULL;
//     }
//     node->test_stmt->name = NULL;
//     node->test_stmt->body = NULL;

//     advance_token(pars); // skip 'test'

//     // expect name
//     if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
//         struct report* err = new_report(
//             SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, //             pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
//         );
//         new_parser_error(pars, err);
//         free_ast(node);
//         return NULL;
//     }
//     node->test_stmt->name = util_strdup(pars->current.literal);
//     advance_token(pars);

//     // expect '{'
//     if(!consume_token(pars, CATEGORY_PAREN, PAR_LBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

//     // parse test block
//     node->test_stmt->body = parse_block(pars);
//     if(!node->test_stmt->body){
//         free_ast(node);
//         return NULL;
//     }

//     // expect '}'
//     if(!consume_token(pars, CATEGORY_PAREN, PAR_RBRACE, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

//     return node;
// }

// struct ast_node* parse_fork(struct parser* pars)
// {
//     struct ast_node* node = new_ast(NODE_FORK);
//     if(!node) return NULL;

//     node->fork_stmt = (struct node_fork*)malloc(sizeof(struct node_fork));
//     if(!node->fork_stmt){
//         free_ast(node);
//         return NULL;
//     }
//     node->fork_stmt->keyw = KW_FORK;
//     node->fork_stmt->name = NULL;
//     node->fork_stmt->body = NULL;

//     advance_token(pars); // skip 'fork'

//     // expect name
//     if(pars->current.category != CATEGORY_LITERAL && pars->current.type_literal != LIT_IDENT){
//         struct report* err = new_report(
//             SEVERITY_ERROR, ERROR_EXPECTED_IDENTIFIER, //             pars->lexer->line, pars->lexer->column, 1, pars->lexer->input
//         );
//         new_parser_error(pars, err);
//         free_ast(node);
//         return NULL;
//     }
//     node->fork_stmt->name = util_strdup(pars->current.literal);
//     advance_token(pars);

//     // expect '{'
//     node->fork_stmt->body = parse_block(pars);
//     if(!node->fork_stmt->body){
//         free_ast(node);
//         return NULL;
//     }

//     return node;
// }

// struct ast_node* parse_solve(struct parser* pars)
// {
//     struct ast_node* node = new_ast(NODE_SOLVE);
//     if(!node) return NULL;

//     node->solve_stmt = (struct node_solve*)malloc(sizeof(struct node_solve));
//     if(!node->solve_stmt){
//         free_ast(node);
//         return NULL;
//     }
//     node->solve_stmt->params = NULL;
//     node->solve_stmt->param_count = 0;
//     node->solve_stmt->param_capacity = 0;
//     node->solve_stmt->body = NULL;

//     advance_token(pars); // skip 'solve'

//     // expect '('
//     if(!consume_token(pars, CATEGORY_PAREN, PAR_LPAREN, ERROR_EXPECTED_PAREN)){ free_ast(node); return NULL;}

//     // parse parameters
//     if(!(pars->current.category == CATEGORY_PAREN && pars->current.type_paren == PAR_RPAREN)){
//         while(1){
//             struct ast_node* param = parse_var_decl(pars);
//             if(!param){
//                 // cleanup
//                 for (size_t i = 0; i < node->solve_stmt->param_count; ++i) free_ast(node->solve_stmt->params[i]);
//                 free(node->solve_stmt->params);
//                 free_ast(node);
//                 return NULL;
//             }

//             // check capacity
//             if(node->solve_stmt->param_count >= node->solve_stmt->param_capacity){
//                 size_t new_cap = node->solve_stmt->param_capacity == 0 ? 4 : node->solve_stmt->param_capacity * 2;
//                 struct ast_node** new_arr = (struct ast_node**)realloc(node->solve_stmt->params, new_cap * sizeof(struct ast_node*));
//                 if(!new_arr){
//                     free_ast(param);
//                     for (size_t i = 0; i < node->solve_stmt->param_count; ++i) free_ast(node->solve_stmt->params[i]);
//                     free(node->solve_stmt->params);
//                     free_ast(node);
//                     return NULL;
//                 }
//                 node->solve_stmt->params = new_arr;
//                 node->solve_stmt->param_capacity = new_cap;
//             }
//             node->solve_stmt->params[node->solve_stmt->param_count++] = param;

//             // consume comma
//             if(pars->current.category == CATEGORY_OPERATOR && pars->current.type_operator == OPER_COMMA){ advance_token(pars);  continue; }

//             break;
//         }
//     }

//     // expect ')'
//     if(!consume_token(pars, CATEGORY_PAREN, PAR_RPAREN, ERROR_EXPECTED_PAREN)){
//         for (size_t i = 0; i < node->solve_stmt->param_count; ++i) free_ast(node->solve_stmt->params[i]);
//         free(node->solve_stmt->params);
//         free_ast(node);
//         return NULL;
//     }

//     // parse body
//     node->solve_stmt->body = parse_block(pars);
//     if(!node->solve_stmt->body){
//         free_ast(node);
//         return NULL;
//     }

//     return node;
// }

// struct ast_node* parse_simulate(struct parser* pars)
// {
//     struct ast_node* node = new_ast(NODE_SIMULATE);
//     if(!node) return NULL;

//     node->simulate_stmt = (struct node_simulate*)malloc(sizeof(struct node_simulate));
//     if(!node->simulate_stmt){
//         free_ast(node);
//         return NULL;
//     }
//     node->simulate_stmt->keyw = KW_SIMULATE;
//     node->simulate_stmt->body = NULL;

//     advance_token(pars); // skip 'simulate'

//     // parse body
//     node->simulate_stmt->body = parse_block(pars);
//     if(!node->simulate_stmt->body){
//         free_ast(node);
//         return NULL;
//     }

//     return node;
// }
