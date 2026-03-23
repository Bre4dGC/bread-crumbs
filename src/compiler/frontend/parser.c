#include <stddef.h>     // size_t

#include "core/ds/arena.h"              // arena_t
#include "core/lang/diagnostic.h"       // diagnostic_t

#include "compiler/frontend/ast.h"      // ast_t, node_t
#include "compiler/frontend/parser.h"   // parser_t
#include "compiler/frontend/parser/decl.h"  // parse_decl_func, parse_decl_struct, etc.
#include "compiler/frontend/parser/stmt.h"  // parse_stmt_if, parse_stmt_while, etc.

#ifdef DEBUG
#include "core/lang/debug.h"    // print_ast
#endif

parse_func_t parse_table[] = {
    [KW_IF]       = parse_stmt_if,
    [KW_WHILE]    = parse_stmt_while,
    [KW_FOR]      = parse_stmt_for,
    [KW_MATCH]    = parse_stmt_match,
    [KW_TRY]      = parse_stmt_trycatch,
    [KW_BREAK]    = parse_stmt_jump,
    [KW_CONTINUE] = parse_stmt_jump,
    [KW_RETURN]   = parse_stmt_jump,

    [KW_FUNC]     = parse_decl_func,
    [KW_STRUCT]   = parse_decl_struct,
    [KW_ENUM]     = parse_decl_enum,
    [KW_TRAIT]    = parse_decl_trait,
    [KW_IMPL]     = parse_decl_impl,
    [KW_TYPE]     = parse_decl_type,
    [KW_MODULE]   = parse_decl_module,
    [KW_IMPORT]   = parse_decl_import,
};

const size_t PARSE_TABLE_LENGTH = sizeof(parse_table)/sizeof(parse_table[0]);

parser_t* new_parser(compiler_context_t* ctx, lexer_t* lexer)
{
    parser_t* parser = arena_alloc(ctx->memory.phase_arena, sizeof(parser_t), alignof(parser_t));
    if(!parser) return NULL;
    parser->token.current = next_token(lexer);
    parser->token.next = next_token(lexer);
    parser->lexer = lexer;
    parser->ctx = ctx;
    return parser;
}

ast_t* parse_program(parser_t* parser)
{
    if(!parser) return NULL;

    parser->ctx->ast = arena_alloc_default(parser->ctx->memory.perm_arena, sizeof(ast_t));
    if(!parser->ctx->ast) return NULL;

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

        if(!add_stmt_block(parser, parser->ctx->ast->nodes, stmt)) goto cleanup;

        // optionally consume ';'
        if(check_token(parser, CAT_OPERATOR, OPER_SEMICOLON)){
            advance_token(parser);
        }

        parser->ctx->ast->count++;
    }

#ifdef DEBUG
    print_ast(parser->ctx->ast->nodes, 0);
#endif

    return parser->ctx->ast;

cleanup:
    free_ast(parser->ctx->ast);
    return NULL;
}

bool check_token(parser_t* parser, enum category_tag category, int type)
{
    return parser->token.current.category == category && parser->token.current.type == type;
}

bool is_eof(const token_t token)
{
    return token.category == CAT_SERVICE && token.type == SERV_EOF;
}

void set_node_loc(node_t* node, parser_t* parser)
{
    if(!node || !parser) return;
    node->loc = parser->lexer->loc;
}

void set_node_len(node_t* node, parser_t* parser, size_t start_pos)
{
    if(!node || !parser) return;
    size_t end_pos = parser->lexer->loc.offset;
    if(end_pos > start_pos){
        node->loc.length = end_pos - start_pos;
    }
    else {
        node->loc.length = 1;
    }
}

size_t get_lexer_pos(parser_t* parser)
{
    return parser ? parser->lexer->loc.offset : 0;
}

void advance_token(parser_t* parser)
{
    if(!parser || is_eof(parser->token.next)) return;
    parser->token.current = parser->token.next;
    parser->token.next = next_token(parser->lexer);
}

bool consume_token(parser_t* parser, node_t* node, const enum category_tag expec_category, const int expec_type, const enum report_code err)
{
    if(!parser) return false;

    if(parser->token.current.category != expec_category){
        add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, err, node->loc);
        return false;
    }

    if(expec_type >= 0){
        int actual_type = parser->token.current.type;

        if(actual_type != expec_type){
            add_report(parser->ctx->reports, parser->ctx->src_manager.current, SEV_ERR, err, node->loc);
            return false;
        }
    }

    advance_token(parser);
    return true;
}
