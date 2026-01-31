#include "compiler/frontend/parser/stmt.h"

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

bool add_stmt_block(parser_t* parser, node_t* node, node_t* stmt)
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

node_t* parse_stmt_block(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_BLOCK);
    if(!node) return NULL;
    set_node_location(node, parser);

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

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_jump_stmt(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
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
        set_node_location(node, parser);
        set_node_length(node, parser, start_pos);
    }
    return node;
}

node_t* parse_if(parser_t* parser){
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_IF);
    if(!node) return NULL;
    set_node_location(node, parser);

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
    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_while(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_WHILE);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'while'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    node->while_stmt->condition = parse_expr(parser);
    if(!node->while_stmt->condition) return NULL;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->while_stmt->body = parse_block(parser);
    }
    else {
        node->while_stmt->body = parse_stmt(parser);
    }

    if(!node->while_stmt->body) return NULL;

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_for(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_FOR);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'for'

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse init statement
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_stmt->init = parse_expr(parser);
        if(!node->for_stmt->init) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        return NULL;
    }

    // parse condition
    if(!check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
        node->for_stmt->condition = parse_expr(parser);
        if(!node->for_stmt->condition) return NULL;
    }

    // expect ';'
    if(!consume_token(parser, CAT_OPERATOR, OPER_SEMICOLON, ERR_EXPEC_DELIM)){
        return NULL;
    }

    // parse update statement
    if(!check_token(parser,  CAT_PAREN, PAR_RPAREN)){
        node->for_stmt->update = parse_expr(parser);
        if(!node->for_stmt->update) return NULL;
    }

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body (can be a block or a single statement)
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->for_stmt->body = parse_block(parser);
    }
    else {
        node->for_stmt->body = parse_stmt(parser);
    }

    if(!node->for_stmt->body) return NULL;

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_match(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_MATCH);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'match'

    // parse target expression
    node->match_stmt->target = parse_expr(parser);
    if(!node->match_stmt->target) return NULL;

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse cases until '}'
    while(!check_token(parser, CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        // create new case node
        node_t* case_node = new_node(parser->ast, NODE_CASE);
        if(!case_node) return NULL;

        // create case struct
        case_node->case_stmt = (struct node_case*)arena_alloc(parser->ast, sizeof(struct node_case), alignof(struct node_case));
        if(!case_node->case_stmt) return NULL;

        // parse case condition
        case_node->case_stmt->condition = parse_expr(parser);
        if(!case_node->case_stmt->condition) return NULL;

        // expect '->'
        if(!consume_token(parser, CAT_OPERATOR, OPER_ARROW, ERR_EXPEC_OPER)){
            return NULL;
        }

        // parse case body (can be a block or a single statement)
        case_node->case_stmt->body = parse_stmt(parser);
        if(!case_node->case_stmt->body) return NULL;

        // grow array if needed
        if(node->match_stmt->block.count >= node->match_stmt->block.capacity){
            size_t new_cap = node->match_stmt->block.capacity == 0 ? 4 : node->match_stmt->block.capacity * 2;
            node_t** new_cases = (node_t**)arena_alloc_array(parser->ast, sizeof(node->match_stmt->block.elems[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_cases) return NULL;
            node->match_stmt->block.elems = new_cases;
            node->match_stmt->block.capacity = new_cap;
        }
        node->match_stmt->block.elems[node->match_stmt->block.count++] = case_node;
    }

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_trait(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_TRAIT);
    if(!node) return NULL;
    set_node_location(node, parser);

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
        return NULL;
    }

    // parse body
    node->trait_decl->body = parse_block(parser);
    if(!node->trait_decl->body) return NULL;

    // expect '}'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_type(parser_t* parser)
{
    return NULL;
}

node_t* parse_impl(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_IMPL);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'impl'

    // expect trait name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->impl_decl->trait_name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->impl_decl->trait_name.data) return NULL;
    advance_token(parser);

    // optional 'for' clause
    if(check_token(parser, CAT_KEYWORD, KW_FOR)){
        advance_token(parser); // skip 'for'

        // expect struct name
        if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }
        node->impl_decl->struct_name = new_string(parser->string_pool, parser->token.current.literal);
        if(!node->impl_decl->struct_name.data) return NULL;
        advance_token(parser);
    }
    else {
        node->impl_decl->struct_name = (string_t){0};
    }

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse body
    node->impl_decl->body = parse_block(parser);
    if(!node->impl_decl->body) return NULL;

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_trycatch(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_TRYCATCH);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'try'

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse try block
    node->trycatch_stmt->try_block = parse_block(parser);
    if(!node->trycatch_stmt->try_block) return NULL;

    // expect 'catch'
    if(!consume_token(parser, CAT_KEYWORD, KW_CATCH, ERR_EXPEC_KEYWORD)){
        return NULL;
    }

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // TODO: parse catch exception variable
    // for now, just skip the identifier if present
    if(check_token(parser, CAT_LITERAL, LIT_IDENT)){
        advance_token(parser);
    }

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

    // optional 'finally' block
    if(check_token(parser, CAT_KEYWORD, KW_FINALLY)){
        advance_token(parser);

        // expect '{'
        if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
            return NULL;
        }

        node->trycatch_stmt->finally_block = parse_block(parser);
        if(!node->trycatch_stmt->finally_block) return NULL;
    }
    else {
        node->trycatch_stmt->finally_block = NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_module(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_MODULE);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'module'

    // expect module name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->module_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->module_decl->name.data) return NULL;
    advance_token(parser);

    // optional module body
    if(check_token(parser, CAT_PAREN, PAR_LBRACE)){
        node->module_decl->body = parse_block(parser);
        if(!node->module_decl->body) return NULL;
    }
    else {
        node->module_decl->body = NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_import(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_IMPORT);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'import'

    // parse module path
    do {
        // expect module name component
        if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        if(node->import_decl->count >= node->import_decl->capacity){
            size_t new_cap = node->import_decl->capacity == 0 ? 4 : node->import_decl->capacity * 2;
            string_t* new_modules = (string_t*)arena_alloc_array(parser->ast, sizeof(node->import_decl->modules[0]), new_cap * sizeof(string_t), alignof(string_t));
            if(!new_modules) return NULL;
            node->import_decl->modules = new_modules;
            node->import_decl->capacity = new_cap;
        }

        // store module name component
        string_t module_name = new_string(parser->string_pool, parser->token.current.literal);
        if(!module_name.data) return NULL;
        node->import_decl->modules[node->import_decl->count++] = module_name;

        advance_token(parser);

        // check for '.' to continue path
        if(check_token(parser, CAT_OPERATOR, OPER_DOT)){
            advance_token(parser);
            continue;
        }
        else {
            break;
        }
    } while (true);

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_special(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    int spec_kind = parser->token.current.type;
    enum node_kind node_kind = (spec_kind == KW_NAMEOF) ? NODE_NAMEOF : NODE_TYPEOF;

    node_t* node = new_node(parser->ast, node_kind);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip special keyword

    // expect '('
    if(!consume_token(parser, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse the expression inside
    node_t* expr = parse_expr(parser);
    if(!expr){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_EXPR, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }

    // store expression as string representation (for now)
    // tODO: This should store the actual expression node
    node->special_stmt->content = new_string(parser->string_pool, "");
    if(!node->special_stmt->content.data) return NULL;
    node->special_stmt->type = spec_kind;

    // expect ')'
    if(!consume_token(parser, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)){
        return NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}
