#include "compiler/frontend/parser/expr.h"
#include "compiler/frontend/parser.h"

node_t* parse_expr(parser_t* parser)
{
    if(!parser) return NULL;

    switch(parser->token.current.category){
        case CAT_OPERATOR: return parse_expr_operator(parser);
        case CAT_KEYWORD:  return parse_expr_keyword(parser);
        case CAT_PAREN:    return parse_expr_paren(parser);
        case CAT_MODIFIER: return parse_decl_var(parser);
        case CAT_LITERAL:  return parse_expr_binop(parser, 0);
        default: return NULL;
    }

    return NULL;
}

node_t* parse_expr_keyword(parser_t* parser)
{
    const int kw = parser->token.current.type;

    if(kw < 0 || (size_t)kw >= PARSE_TABLE_LENGTH){
        return NULL;
    }

    parse_func_t func = parse_table[kw];

    if(!func) return NULL;

    return func(parser);
}

node_t* parse_expr_paren(parser_t* parser)
{
    switch(parser->token.current.type){
        case PAR_LBRACE:   return parse_stmt_block(parser);
        case PAR_LBRACKET: return parse_decl_array(parser);
        case PAR_LPAREN:
            advance_token(parser); // skip '('

            node_t* node = parse_expr(parser);
            if(!node) return NULL;

            if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
                return NULL;
            }
            return node;
        default: return NULL;
    }
}

node_t* parse_expr_literal(parser_t* parser)
{
    node_t* node = new_node(parser->ast, NODE_LITERAL);
    if(!node) return NULL;

    node->lit->value = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->lit->value.data) return NULL;
    node->lit->type = parser->token.current.type;

    advance_token(parser);
    return node;
}

node_t* parse_expr_operator(parser_t* parser)
{
    bool is_unary = (parser->token.next.category == CAT_LITERAL) ||
                    (parser->token.next.category == CAT_PAREN    &&
                    (parser->token.next.type     == PAR_LPAREN   ||
                     parser->token.next.type     == PAR_LBRACE   ||
                     parser->token.next.type     == PAR_LBRACKET));

    return is_unary ? parse_expr_unaryop(parser) : parse_expr_binop(parser, 0);
}

node_t* parse_expr_func_call(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_CALL);
    if(!node) return NULL;
    set_node_location(node, parser);

    // extract function name before consuming token
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        return NULL;
    }
    node->func_call->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->func_call->name.data) return NULL;
    advance_token(parser);

    // parse arguments if '(' is present
    if(check_token(parser, CAT_PAREN, PAR_LPAREN)){
        advance_token(parser); // skip '('

        // parse arguments until ')'
        while (!check_token(parser, CAT_PAREN, PAR_RPAREN)){
            node_t* arg = parse_expr(parser);
            if(!arg) return NULL;

            // grow array if needed
            if(node->func_call->args.count >= node->func_call->args.capacity){
                size_t new_capacity = node->func_call->args.capacity == 0 ? 4 : node->func_call->args.capacity * 2;
                node_t** new_args = (node_t**)arena_alloc_array(parser->ast, sizeof(node_t*), new_capacity, alignof(node_t*));
                if(!new_args) return NULL;
                node->func_call->args.elems = new_args;
                node->func_call->args.capacity = new_capacity;
            }

            node->func_call->args.elems[node->func_call->args.count++] = arg;

            if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
                advance_token(parser); // skip ','
            }
            else {
                break;
            }
        }

        // expect closing ')'
        if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
            return NULL;
        }
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_expr_var_ref(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_REF);
    if(!node) return NULL;
    set_node_location(node, parser);

    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }

    node->var_ref->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->var_ref->name.data) return NULL;

    advance_token(parser);

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_expr_primary(parser_t* parser)
{
    switch(parser->token.current.category){
        case CAT_LITERAL:
            if(parser->token.current.type == LIT_IDENT){
                if(parser->token.next.category == CAT_PAREN && parser->token.next.type == PAR_LPAREN){
                    return parse_expr_func_call(parser);
                }
                return parse_expr_var_ref(parser);
            }
            else {
                size_t start_pos = get_lexer_position(parser);
                node_t* node = new_node(parser->ast, NODE_LITERAL);
                if(!node) return NULL;
                set_node_location(node, parser);
                node->lit->value = new_string(parser->string_pool, parser->token.current.literal);
                node->lit->type = parser->token.current.type;
                advance_token(parser);
                set_node_length(node, parser, start_pos);
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
                return parse_decl_array(parser);
            }
            break;
        case CAT_OPERATOR:
            // check for unary operators
            if(parser->token.current.type == OPER_PLUS
            || parser->token.current.type == OPER_MINUS
            || parser->token.current.type == OPER_NOT)
            {
                return parse_expr_unaryop(parser);
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

node_t* parse_expr_postfix(parser_t* parser)
{
    node_t* expr = parse_expr_primary(parser);
    if(!expr) return NULL;

    while(parser->token.current.category == CAT_OPERATOR){
        enum category_operator op = parser->token.current.type;

        if(op == OPER_INCREM || op == OPER_DECREM){
            size_t start_pos = get_lexer_position(parser) - expr->length;
            node_t* postfix = new_node(parser->ast, NODE_UNARYOP);
            if(!postfix) return NULL;

            postfix->loc = expr->loc;
            postfix->unaryop->right = expr;
            postfix->unaryop->is_postfix = true;
            postfix->unaryop->operator = op;

            advance_token(parser);
            set_node_length(postfix, parser, start_pos);

            expr = postfix;
        }
        else break;
    }
    return expr;
}

node_t* parse_expr_binop(parser_t* parser, int min_precedence)
{
    node_t* left = parse_expr_postfix(parser);
    if(!left) return NULL;

    size_t expr_start_pos = get_lexer_position(parser) - left->length;

    while(parser->token.current.category == CAT_OPERATOR){
        enum category_operator op_type = parser->token.current.type;
        if(op_type == OPER_SEMICOLON || op_type == OPER_COMMA) break;

        int precedence = get_operator_precedence(op_type);
        if(precedence == 0 || precedence < min_precedence) break;

        #ifdef DEBUG
        const char* op_literal = parser->token.current.literal;
        #endif

        advance_token(parser);

        // calculate next minimum precedence
        int next_min_prec;
        if(is_right_associative(op_type)) next_min_prec = precedence;
        else next_min_prec = precedence + 1;

        node_t* right = parse_expr_binop(parser, next_min_prec);
        if(!right){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_EXPR, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        node_t* node = new_node(parser->ast, NODE_BINOP);
        if(!node) return NULL;

        node->binop->left = left;
        node->binop->right = right;
        node->binop->operator = op_type;
    #ifdef DEBUG
        node->binop->lit = op_literal;
    #endif
        // use location from left operand
        node->loc = left->loc;

        left = node;
    }

    // update length to span the entire expression
    if(left){
        size_t current_pos = get_lexer_position(parser);
        if(current_pos > expr_start_pos){
            left->length = current_pos - expr_start_pos;
        }
    }
    return left;
}

node_t* parse_expr_unaryop(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_UNARYOP);
    if(!node) return NULL;
    set_node_location(node, parser);

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
        node->unaryop->right = parse_expr_postfix(parser);
    }
    else {
        node->unaryop->right = parse_expr_primary(parser);
    }

    if(!node->unaryop->right) return NULL;

    set_node_length(node, parser, start_pos);
    return node;
}
