#include "compiler/frontend/parser/stmt.h"
#include "compiler/frontend/parser/expr.h"
#include "compiler/frontend/parser/decl.h"

node_t* parse_stmt(parser_t* parser)
{
    if(!parser) return NULL;

    // skip empty statements
    if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        advance_token(parser);
        return NULL;
    }

    if(parser->token.current.category == CAT_KEYWORD){
        return parse_stmt_keyword(parser);
    }
    else if(parser->token.current.category == CAT_MODIFIER){
        return parse_decl_var(parser);
    }
    else if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        return parse_stmt_block(parser);
    }

    return parse_expr(parser);
}

node_t* parse_stmt_keyword(parser_t* parser)
{
    if(!parser) return NULL;
    int kw = parser->token.current.type;
    if(kw < 0 || (size_t)kw >= PARSE_TABLE_LENGTH) return NULL;
    parse_func_t func = parse_table[kw];
    if(!func) return NULL;
    return func(parser);
}

bool add_stmt_block(parser_t* parser, node_t* node, node_t* stmt)
{
    if(node->kind != NODE_BLOCK) return false;

    if(node->block->statement.count >= node->block->statement.capacity){
        size_t new_capacity = node->block->statement.capacity == 0 ? 4 : node->block->statement.capacity * 2;
        node_t** new_statements = arena_alloc_array(parser->ast, sizeof(node_t*), new_capacity, alignof(node_t*));
        if(!new_statements) return false;

        node->block->statement.elems = new_statements;
        node->block->statement.capacity = new_capacity;
    }

    node->block->statement.elems[node->block->statement.count++] = stmt;
    return true;
}

node_t* parse_stmt_block(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_BLOCK);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip '{'

    // parse statements until '}'
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
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
        if(!add_stmt_block(parser, node, stmt)) return NULL;

        // optionally consume ';'
        if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
            advance_token(parser);
        }
    }

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_jump(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    int type = parser->token.current.type;
    node_t* node = NULL;

    advance_token(parser); // skip keyword

    switch(type){
        case KW_BREAK:
            node = new_node(parser->ast, NODE_BREAK);
            break;
        case KW_CONTINUE:
            node = new_node(parser->ast, NODE_CONTINUE);
            break;
        case KW_RETURN:
            node = new_node(parser->ast, NODE_RETURN);
            if(!node) return NULL;
            if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON) &&
               !check_token(parser, CAT_PAREN, PAR_RBRACE)){
                node->return_stmt->body = parse_expr(parser);
            }
            break;
        default:
            return NULL;
    }

    if(node){
        set_node_loc(node, parser);
        set_node_len(node, parser, start_pos);
    }
    return node;
}

node_t* parse_stmt_if(parser_t* parser){
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_IF);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'if'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    node->if_stmt->condition = parse_expr(parser);
    if(!node->if_stmt->condition) return NULL;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // expect '{'
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->if_stmt->then_block = parse_stmt_block(parser);
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
                add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
                return NULL;
            }

            node_t* elif_condition = parse_expr(parser);
            if(!elif_condition) return NULL;

            // expect ')'
            if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
                add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
                return NULL;
            }

            node_t* elif_body;

            // expect '{'
            if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
                elif_body = parse_stmt_block(parser);
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
                node->if_stmt->else_block = parse_stmt_block(parser);
            }
            else {
                node->if_stmt->else_block = parse_stmt(parser);
            }

            if(!node->if_stmt->else_block) return NULL;
            break;
        }
    }
    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_while(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_WHILE);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'while'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    node->while_stmt->condition = parse_expr(parser);
    if(!node->while_stmt->condition) return NULL;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->while_stmt->body = parse_stmt_block(parser);
    }
    else {
        node->while_stmt->body = parse_stmt(parser);
    }

    if(!node->while_stmt->body) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_for(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_FOR);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'for'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse init statement
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_stmt->init = parse_expr(parser);
        if(!node->for_stmt->init) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_DELIM, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse condition
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_stmt->condition = parse_expr(parser);
        if(!node->for_stmt->condition) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_DELIM, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse update statement
    if(!check_token(parser,  CAT_PAREN, PAR_RPAREN)){
        node->for_stmt->update = parse_expr(parser);
        if(!node->for_stmt->update) return NULL;
    }

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->for_stmt->body = parse_stmt_block(parser);
    }
    else {
        node->for_stmt->body = parse_stmt(parser);
    }

    if(!node->for_stmt->body) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_case(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_CASE);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'case'

    node->case_stmt->condition = parse_expr(parser);
    if(!node->case_stmt->condition) return NULL;

    // expect '->'
    if(!consume_token(parser, CAT_OPERATOR, OPER_ARROW, ERR_EXPEC_OPER)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_OPER, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->case_stmt->body = parse_stmt_block(parser);
    }
    else {
        node->case_stmt->body = parse_stmt(parser);
    }

    if(!node->case_stmt->body) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_match(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_MATCH);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'match'

    // parse target expression
    node->match_stmt->target = parse_expr(parser);
    if(!node->match_stmt->target) return NULL;

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse cases until '}'
    while(!check_token(parser, CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        node_t* case_node = parse_stmt_case(parser);
        if(!case_node){
            if(parser->token.current.category == CAT_OPERATOR && parser->token.current.type == OPER_ARROW){
                advance_token(parser);
            }
            else {
                advance_token(parser);
            }
            continue;
        }

        // add to match statement
        if(!node->match_stmt->block.elems){
            node->match_stmt->block.elems = arena_alloc_array(parser->ast, sizeof(node_t*), 4, alignof(node_t*));
            if(!node->match_stmt->block.elems) return NULL;
            node->match_stmt->block.capacity = 4;
        }
        else if(node->match_stmt->block.count >= node->match_stmt->block.capacity){
            size_t new_capacity = node->match_stmt->block.capacity * 2;
            node_t** new_cases = arena_alloc_array(parser->ast, sizeof(node_t*), new_capacity, alignof(node_t*));
            if(!new_cases) return NULL;

            for(size_t i = 0; i < node->match_stmt->block.count; i++){
                new_cases[i] = node->match_stmt->block.elems[i];
            }
            node->match_stmt->block.elems = new_cases;
            node->match_stmt->block.capacity = new_capacity;
        }
        node->match_stmt->block.elems[node->match_stmt->block.count++] = case_node;
    }

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_trait(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_TRAIT);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'trait'

    // expect name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->trait_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->trait_decl->name.data) return NULL;
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse body
    node->trait_decl->body = parse_stmt_block(parser);
    if(!node->trait_decl->body) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_stmt_trycatch(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ast, NODE_TRYCATCH);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'try'

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse try block
    node->trycatch_stmt->try_block = parse_stmt_block(parser);
    if(!node->trycatch_stmt->try_block) return NULL;

    // expect 'catch'
    if(!consume_token(parser, CAT_KEYWORD, KW_CATCH, ERR_EXPEC_KEYWORD)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_KEYWORD, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // TODO: parse catch exception variable
    // for now, just skip the identifier if present
    if(check_token(parser, CAT_LITERAL, LIT_IDENT)){
        advance_token(parser);
    }

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }

    // parse catch block
    node->trycatch_stmt->catch_block = parse_stmt_block(parser);
    if(!node->trycatch_stmt->catch_block) return NULL;

    // optional 'finally' block
    if(check_token(parser, CAT_KEYWORD, KW_FINALLY)){
        advance_token(parser);

        // expect '{'
        if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
            return NULL;
        }

        node->trycatch_stmt->finally_block = parse_stmt_block(parser);
        if(!node->trycatch_stmt->finally_block) return NULL;
    }
    else {
        node->trycatch_stmt->finally_block = NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}
