#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "core/strings.h"
#include "compiler/frontend/ast.h"
#include "compiler/frontend/parser.h"

#ifdef DEBUG
#include "core/common/debug.h"
#endif

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
            if((parser->token.current.category == prev_token.category
            &&  parser->token.current.type == prev_token.type)
            && !is_eof(parser->token.current)
            && !is_eof(parser->token.next))
            {
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

inline bool check_token(parser_t* parser, enum category_tag category, int type)
{
    return parser->token.current.category == category && parser->token.current.type == type;
}

inline bool is_eof(const token_t token)
{
    return token.category == CAT_SERVICE && token.type == SERV_EOF;
}

inline void set_node_location(node_t* node, parser_t* parser)
{
    if(!node || !parser) return;
    node->loc = parser->lexer->loc;
}

inline void set_node_length(node_t* node, parser_t* parser, size_t start_pos)
{
    if(!node || !parser) return;
    size_t end_pos = parser->lexer->pos;
    if(end_pos > start_pos){
        node->length = end_pos - start_pos;
    }
    else {
        node->length = 1;
    }
}

inline size_t get_lexer_position(parser_t* parser)
{
    return parser ? parser->lexer->pos : 0;
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
