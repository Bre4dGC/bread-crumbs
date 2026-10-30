#include "compiler/frontend/parser/decl.h"  // parse_decl_var, parse_decl_type
#include "compiler/frontend/parser/expr.h"  // parse_expr
#include "compiler/frontend/parser/stmt.h"  // parse_stmt_block
#include "core/lang/source.h"

node_t* parse_decl_var(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_VARIABLE);
    if(!node) return NULL;
    set_node_loc(node, parser);

    // expect modifier
    if(parser->token.current.category != CAT_MODIFIER){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_KEYWORD, node->loc);
    }
    node->var_decl->modif = parser->token.current.type;
    advance_token(parser);

    // expect identifier
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }
    node->var_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->var_decl->name.data) return NULL;
    advance_token(parser);

    // optional type annotation
    if(check_token(parser, CAT_OPERATOR, OPER_COLON)){
        advance_token(parser);
        if(parser->token.current.category != CAT_DATATYPE && !check_token(parser, CAT_LITERAL, LIT_IDENT)){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_TYPE, loc_copy(node->loc, 1));
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
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_EXPR, node->loc);
            return NULL;
        }
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_type(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_TYPE);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'type'

    // expect type name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }
    node->type_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->type_decl->name.data) return NULL;
    advance_token(parser);

    // expect ':'
    if(!consume_token(parser, node, CAT_OPERATOR, OPER_COLON, ERR_EXPEC_OPER)) return NULL;

    // expect type body (currently only struct or enum)
    if(check_token(parser, CAT_KEYWORD, KW_STRUCT)){
        node->type_decl->body = parse_decl_struct(parser);
    }
    else if(check_token(parser, CAT_KEYWORD, KW_ENUM)){
        node->type_decl->body = parse_decl_enum(parser);
    }
    else {
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_TYPE, node->loc);
        return NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_array(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_ARRAY);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip '['

    // parse elements (comma separated)
    while(!check_token(parser, CAT_PAREN, PAR_RBRACKET)){
        node_t* element = parse_expr(parser);
        if(!element) return NULL;

        if(node->array_decl->count >= node->array_decl->capacity){
            size_t new_cap = node->array_decl->capacity == 0 ? 4 : node->array_decl->capacity * 2;
            node_t** new_arr = arena_alloc_array(parser->ctx->ast->arena, sizeof(node_t*), new_cap, alignof(node_t*));
            if(!new_arr) return NULL;

            for(size_t i = 0; i < node->array_decl->count; i++){
                new_arr[i] = node->array_decl->elements[i];
            }

            node->array_decl->elements = new_arr;
            node->array_decl->capacity = new_cap;
        }
        node->array_decl->elements[node->array_decl->count++] = element;

        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
            advance_token(parser); // consume ','
            continue;
        }
        break;
    }

    // expect ']'
    if(!consume_token(parser, node, CAT_PAREN, PAR_RBRACKET, ERR_EXPEC_PAREN)) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_param(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_VARIABLE);
    if(!node) return NULL;
    set_node_loc(node, parser);

    // expect identifier
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }
    node->var_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->var_decl->name.data) return NULL;
    advance_token(parser);

    // expect ':'
    if(!consume_token(parser, node, CAT_OPERATOR, OPER_COLON, ERR_EXPEC_OPER)) return NULL;

    // expect datatype
    if(parser->token.current.category != CAT_DATATYPE){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_TYPE, node->loc);
        return NULL;
    }
    node->var_decl->dtype = parser->token.current.type;
    advance_token(parser);

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_func(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_FUNC);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'func'

    // expect function name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }

    node->func_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->func_decl->name.data) return NULL;
    advance_token(parser);

    // expect '('
    if(!consume_token(parser, node, CAT_PAREN, PAR_LPAREN, ERR_EXPEC_PAREN)) return NULL;

    // parsing params until ')'
    if(!check_token(parser, CAT_PAREN, PAR_RPAREN)){
        while(true){
            node_t* param_decl = parse_decl_param(parser);
            if(!param_decl) return NULL;

            // ensure parameter node is a variable
            if(param_decl->kind != NODE_VARIABLE){
                add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_PARAM, node->loc);
                return NULL;
            }

            if(!nodes_push(&node->func_decl->param_decl, parser->ctx->ast->arena, param_decl)) return NULL;

            // consume ','
            if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
                advance_token(parser);
                continue;
            }
            break;
        }
    }

    // consume ')'
    if(!consume_token(parser, node, CAT_PAREN, PAR_RPAREN, ERR_EXPEC_PAREN)) return NULL;

    // optional return type
    if(check_token(parser, CAT_OPERATOR, OPER_COLON)){
        advance_token(parser);

        // expect datatype
        if(parser->token.current.category != CAT_DATATYPE){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_TYPE, node->loc);
            return NULL;
        }

        node->func_decl->return_type = parser->token.current.type;
        advance_token(parser);
    }

    // expect function body (block)
    node->func_decl->body = parse_stmt_block(parser);
    if(!node->func_decl->body) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_struct(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_STRUCT);
    if(!node) return NULL;
    set_node_loc(node, parser);

    advance_token(parser); // skip 'struct'

    // expect '{'
    if(!consume_token(parser, node, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)) return NULL;

    // parse struct members
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE)){

        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_PAREN, node->loc);
            return NULL;
        }

        // parse member variable declaration
        node_t* member = parse_decl_var(parser);
        if(!member) return NULL;

        // add member
        if(!nodes_push(&node->struct_decl->member, parser->ctx->ast->arena, member)) return NULL;

        // optional comma separator
        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
            advance_token(parser);
        }
    }

    // expect '}'
    if(!consume_token(parser, node, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_variant(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_VARIANT);
    if(!node) return NULL;
    set_node_loc(node, parser);

    // expect member name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }
    node->variant_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->variant_decl->name.data) return NULL;
    advance_token(parser);

    // optional value assignment
    if(check_token(parser, CAT_OPERATOR, OPER_ASSIGN)){
        advance_token(parser);
        node->variant_decl->value = parse_expr(parser);
        if(!node->variant_decl->value){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_EXPR, node->loc);
            return NULL;
        }
    }
    else {
        node->variant_decl->value = NULL;
    }

    set_node_len(node, parser, start_pos);
    return node;
}

node_t* parse_decl_enum(parser_t* parser)
{
    size_t start_pos = get_lexer_pos(parser);
    node_t* node = new_node(parser->ctx->ast->arena, NODE_ENUM);
    if(!node) return NULL;

    set_node_loc(node, parser);

    advance_token(parser); // skip 'enum'

    // expect enum name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
        return NULL;
    }
    node->enum_decl->name = new_string(&parser->ctx->memory.perm_strings, parser->token.current.literal);
    if(!node->enum_decl->name.data) return NULL;
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, node, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)) return NULL;

    // parse enum members
    while(!check_token(parser, CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)) return NULL;

        // expect member name
        if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc);
            return NULL;
        }

        node_t* member = parse_decl_variant(parser);
        if(!member) return NULL;

        // grow array if needed
        if(!nodes_push(&node->enum_decl->member, parser->ctx->ast->arena, member)) return NULL;

        // optional comma separator
        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)) advance_token(parser);
    }

    // expect '}'
    if(!consume_token(parser, node, CAT_PAREN, PAR_RBRACE, ERR_EXPEC_PAREN)) return NULL;

    set_node_len(node, parser, start_pos);
    return node;
}
