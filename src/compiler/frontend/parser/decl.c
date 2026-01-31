#include "compiler/frontend/parser/decl.h"

node_t* parse_var_decl(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_VAR);
    if(!node) return NULL;
    set_node_location(node, parser);

    // expect modifier
    if(parser->token.current.category != CAT_MODIFIER){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_KEYWORD, node->loc, node->length, parser->lexer->input->data);
    }
    node->var_decl->modif = parser->token.current.type;
    advance_token(parser);

    // expect identifier
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
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
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_TYPE, node->loc, node->length, parser->lexer->input->data);
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
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_EXPR, node->loc, node->length, parser->lexer->input->data);
            return NULL;
        }
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_array(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_ARRAY);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip '['

    // parse elements (comma separated)
    while(!check_token(parser, CAT_PAREN, PAR_RBRACKET)){
        node_t* element = parse_expr(parser);
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
            advance_token(parser); // consume ','
            continue;
        }
        break;
    }

    // expect ']'
    if(!consume_token(parser, CAT_PAREN, PAR_RBRACKET, ERR_EXPEC_PAREN)){
        return NULL;
    }

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_var_param(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_PARAM);
    if(!node) return NULL;
    set_node_location(node, parser);

    // expect identifier
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->var_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->var_decl->name.data) return NULL;
    advance_token(parser);

    // expect ':'
    if(!consume_token(parser, CAT_OPERATOR, OPER_COLON, ERR_EXPEC_OPER)){
        return NULL;
    }

    // expect datatype
    if(parser->token.current.category != CAT_DATATYPE){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_TYPE, node->loc, node->length, parser->lexer->input->data);
        return NULL;
    }
    node->var_decl->dtype = parser->token.current.type;
    advance_token(parser);

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_func(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_FUNC);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'func'

    // expect function name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
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

    // parsing params until ')'
    if(!check_token(parser, CAT_PAREN, PAR_RPAREN)){
        while(true){
            node_t* param_decl = parse_var_param(parser);
            if(!param_decl) return NULL;

            // check if param_decl is a variable
            if(param_decl->kind != NODE_PARAM){
                add_report(parser->reports, SEV_ERR, ERR_EXPEC_PARAM, node->loc, node->length, parser->lexer->input->data);
                return NULL;
            }

            // check if there is enough capacity
            if(node->func_decl->param_decl.count >= node->func_decl->param_decl.capacity){
                size_t new_capacity = node->func_decl->param_decl.capacity * 2;
                node_t** new_params = (node_t**)arena_alloc_array(parser->ast, sizeof(node->func_decl->param_decl.elems[0]), new_capacity * sizeof(node_t*), alignof(node_t*));
                if(!new_params) return NULL;
                node->func_decl->param_decl.elems = new_params;
                node->func_decl->param_decl.capacity = new_capacity;
            }

            node->func_decl->param_decl.elems[node->func_decl->param_decl.count++] = param_decl;

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
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_TYPE, node->loc, node->length, parser->lexer->input->data);
            return NULL;
        }

        node->func_decl->return_type = parser->token.current.type;
        advance_token(parser);
    }

    // expect function body (block)
    node->func_decl->body = parse_block(parser);
    if(!node->func_decl->body) return NULL;

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_struct(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_STRUCT);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'struct'

    // expect struct name
    if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
        add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
        return NULL;
    }
    node->struct_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    if(!node->struct_decl->name.data) return NULL;
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse struct members
    while(!check_token(parser,  CAT_PAREN, PAR_RBRACE)){

        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_PAREN, node->loc, node->length, parser->lexer->input->data);
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

    set_node_length(node, parser, start_pos);
    return node;
}

node_t* parse_enum(parser_t* parser)
{
    size_t start_pos = get_lexer_position(parser);
    node_t* node = new_node(parser->ast, NODE_ENUM);
    if(!node) return NULL;
    set_node_location(node, parser);

    advance_token(parser); // skip 'enum'

    // expect enum name
    if(!consume_token(parser, CAT_LITERAL, LIT_IDENT, ERR_EXPEC_IDENT)){
        return NULL;
    }
    node->enum_decl->name = new_string(parser->string_pool, parser->token.current.literal);
    advance_token(parser);

    // expect '{'
    if(!consume_token(parser, CAT_PAREN, PAR_LBRACE, ERR_EXPEC_PAREN)){
        return NULL;
    }

    // parse enum members
    while(!check_token(parser, CAT_PAREN, PAR_RBRACE))
    {
        // check for EOF
        if(is_eof(parser->token.current) || is_eof(parser->token.next)){
            return NULL;
        }

        // expect member name
        if(!check_token(parser, CAT_LITERAL, LIT_IDENT)){
            add_report(parser->reports, SEV_ERR, ERR_EXPEC_IDENT, parser->lexer->loc, DEFAULT_LEN, parser->lexer->input->data);
            return NULL;
        }

        // create enum member node
        node_t* member = new_node(parser->ast, NODE_MEMBER);
        if(!member) return NULL;

        member->member_decl = (struct node_member*)arena_alloc(parser->ast, sizeof(struct node_member), alignof(struct node_member));
        if(!member->member_decl) return NULL;

        member->member_decl->name = new_string(parser->string_pool, parser->token.current.literal);
        if(!member->member_decl->name.data) return NULL;
        member->member_decl->value = NULL;

        advance_token(parser);

        // optional value assignment
        if(check_token(parser, CAT_OPERATOR, OPER_ASSIGN)){
            advance_token(parser);
            member->member_decl->value = parse_expr(parser);
            if(!member->member_decl->value) return NULL;
        }

        // grow array if needed
        if(node->enum_decl->member.count >= node->enum_decl->member.capacity){
            size_t new_cap = node->enum_decl->member.capacity == 0 ? 4 : node->enum_decl->member.capacity * 2;
            node_t** new_members = (node_t**)arena_alloc_array(parser->ast, sizeof(node->enum_decl->member.elems[0]), new_cap * sizeof(node_t*), alignof(node_t*));
            if(!new_members) return NULL;
            node->enum_decl->member.elems = new_members;
            node->enum_decl->member.capacity = new_cap;
        }
        node->enum_decl->member.elems[node->enum_decl->member.count++] = member;

        // optional comma separator
        if(check_token(parser, CAT_OPERATOR, OPER_COMMA)){
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
