#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "core/strings.h"
#include "compiler/frontend/lexer.h"

#define IDENT_SIZE 16
#define NUM_SIZE 32
#define STR_SIZE 64

#define MAX_IDENT_SIZE 64
#define MAX_NUM_SIZE 128
#define MAX_STR_SIZE 4096

token_t handle_operator(lexer_t* lexer);
token_t handle_paren(lexer_t* lexer);
token_t handle_number(lexer_t* lexer);
token_t handle_ident(lexer_t* lexer);
token_t handle_string(lexer_t* lexer);
void handle_comment(lexer_t* lexer);

string_t read_ident(lexer_t* lexer);
string_t read_number(lexer_t* lexer, enum category_literal lit);
string_t read_string(lexer_t* lexer, char quote_char);
char read_escseq(lexer_t* lexer);

void read_ch(lexer_t *lexer)
{
    if(!lexer || !lexer->input->data) return;

    if(lexer->pos++ >= lexer->input->length){
        lexer->ch = 0;
    }
    else {
        lexer->ch = lexer->input->data[lexer->pos];
        lexer->loc.column++;
    }
}

void skip_whitespace(lexer_t *lexer)
{
    while(true){
        switch(lexer->ch){
            case ' ': case '\t': case '\r':
                read_ch(lexer);
                break;

            case '\n':
                read_ch(lexer);
                lexer->loc.line++;
                lexer->loc.column = 1;
                lexer->boln = lexer->pos;
                break;

            default: return;
        }
    }
}

char peek_ch(const lexer_t* lexer)
{
    if(!lexer || !lexer->input->data) return '\0';

    size_t npos = lexer->pos + 1;
    if(npos < lexer->input->length) return lexer->input->data[npos];
    return '\0';
}

lexer_t* new_lexer(arena_t* arena, string_pool_t* string_pool, report_table_t* reports, string_t* input)
{
    if(!input) return NULL;

    lexer_t* lexer = (lexer_t*)arena_alloc(arena, sizeof(lexer_t), alignof(lexer_t));
    if(!lexer) return NULL;

    lexer->input = input;

    lexer->ch = input->data[0];
    lexer->pos = 0;
    lexer->loc = (location_t){1, 1};
    lexer->boln = 0;
    lexer->balance = 0;
    lexer->string_pool = string_pool;
    lexer->reports = reports;

    return lexer;
}

token_t next_token(lexer_t* lexer)
{
    if(!lexer) return new_token(CAT_SERVICE, SERV_ILLEGAL, "NULL_LEXER");

    while(true){
        skip_whitespace(lexer);
        if(lexer->ch == '#'){
            handle_comment(lexer);
            continue;
        }
        break;
    }

    const char ch_str[2] ={lexer->ch, '\0'};
    token_t token = {0};

    switch(lexer->ch){
        case '+': case '-':
        case '*': case '/':
        case '=': case '!':
        case '<': case '>':
        case '&': case '|':
        case '.': case ',':
        case ':': case ';':
        case '?': case '%':
            token = handle_operator(lexer);
            break;

        case '(': case ')':
        case '{': case '}':
        case '[': case ']':
            token = handle_paren(lexer);
            break;

        case '"': case '\'':
            token = handle_string(lexer);
            break;

        case '\0':
            if(lexer->balance != 0){
                add_report(lexer->reports, SEV_ERR, ERR_UNMAT_PAREN, lexer->loc, DEFAULT_LEN, lexer->input->data);
            }
            token = new_token(CAT_SERVICE, SERV_EOF, "EOF");
            break;

        default:
            if(isalpha(lexer->ch) || lexer->ch == '_'){
                token = handle_ident(lexer);
            }
            else if(isdigit(lexer->ch)){
                token = handle_number(lexer);
            }
            else {
                size_t length = 0;
                while(lexer->ch != '\0' && !isalnum(lexer->ch) && !isspace(lexer->ch)){
                    read_ch(lexer);
                    length++;
                }
                token = new_token(CAT_SERVICE, SERV_ILLEGAL, ch_str);

                add_report(lexer->reports, SEV_ERR, ERR_ILLEG_CHAR, (location_t){lexer->loc.line, lexer->loc.column - length}, length, lexer->input->data);

                read_ch(lexer);
            }
            break;
    }
    return token;
}

token_t handle_operator(lexer_t* lexer)
{
    if(!lexer) return new_token(CAT_SERVICE, SERV_ILLEGAL, "NULL_LEXER");
    const char current = lexer->ch;
    const char next = peek_ch(lexer);

    if(next != '\0'){
        const char potential_op[3] = {current, next, '\0'};
        const token_t *op = find_token(potential_op);
        if(op){
            read_ch(lexer);
            read_ch(lexer);
            return new_token(CAT_OPERATOR, op->type, op->literal);
        }
    }
    read_ch(lexer);

    int op_type;
    char op_char[2] = {current, '\0'};
    switch(current){
        case '+': op_type = OPER_PLUS;      break;
        case '-': op_type = OPER_MINUS;     break;
        case '*': op_type = OPER_ASTERISK;  break;
        case '/': op_type = OPER_SLASH;     break;
        case '%': op_type = OPER_PERCENT;   break;
        case '=': op_type = OPER_ASSIGN;    break;
        case '!': op_type = OPER_NOT;       break;
        case '<': op_type = OPER_LANGLE;    break;
        case '>': op_type = OPER_RANGLE;    break;
        case '.': op_type = OPER_DOT;       break;
        case ',': op_type = OPER_COMMA;     break;
        case ':': op_type = OPER_COLON;     break;
        case ';': op_type = OPER_SEMICOLON; break;
        case '?': op_type = OPER_QUESTION;  break;
        default: {
            string_t stored = new_string(lexer->string_pool, op_char);
            return new_token(CAT_SERVICE, SERV_ILLEGAL, stored.data ? stored.data : op_char);
        }
    }
    string_t stored = new_string(lexer->string_pool, op_char);
    return new_token(CAT_OPERATOR, op_type, stored.data ? stored.data : op_char);
}

token_t handle_ident(lexer_t* lexer)
{
    if(!lexer) return new_token(CAT_SERVICE, SERV_ILLEGAL, "NULL_LEXER");

    string_t ident = read_ident(lexer);
    if(!ident.data){
        while(isalnum(lexer->ch) || lexer->ch == '_') read_ch(lexer);
        return new_token(CAT_SERVICE, SERV_ILLEGAL, "INVALID_IDENT");
    }

    const token_t *kw = find_token(ident.data);

    if(kw) return new_token(kw->category, kw->type, kw->literal);
    else   return new_token(CAT_LITERAL, LIT_IDENT, ident.data);
}

token_t handle_number(lexer_t *lexer)
{
    enum category_literal lit = LIT_NUMBER;

    if(lexer->ch == '0'){
        switch(peek_ch(lexer)){
            case 'x': lit = LIT_HEX; break;
            case 'b': lit = LIT_BIN; break;
            case 'o': lit = LIT_OCT; break;
            case '.': break;
        }
    }
    else lit = LIT_NUMBER;

    string_t num_str = read_number(lexer, lit);
    if(!num_str.data) return new_token(CAT_SERVICE, SERV_ILLEGAL, "BAD_NUMBER");

    token_t token = new_token(CAT_LITERAL, lit, num_str.data);
    return token;
}

token_t handle_paren(lexer_t *lexer)
{
    if(!lexer) return new_token(CAT_SERVICE, SERV_ILLEGAL, "NULL_LEXER");
    if(lexer->ch == '(' || lexer->ch == '{' || lexer->ch == '['){
        lexer->balance++;
    }
    else if(lexer->ch == ')' || lexer->ch == '}' || lexer->ch == ']'){
        if(lexer->balance == 0){
            add_report(lexer->reports, SEV_ERR, ERR_UNMAT_PAREN, lexer->loc, DEFAULT_LEN, lexer->input->data);
        }
        else {
            lexer->balance--;
        }
    }

    enum category_paren type = 0;
    switch(lexer->ch){
        case '(': type = PAR_LPAREN; break;
        case ')': type = PAR_RPAREN; break;
        case '{': type = PAR_LBRACE; break;
        case '}': type = PAR_RBRACE; break;
        case '[': type = PAR_LBRACKET; break;
        case ']': type = PAR_RBRACKET; break;
    }
    char paren_str[2] = {lexer->ch, '\0'};
    string_t stored = new_string(lexer->string_pool, paren_str);
    const token_t token = new_token(CAT_PAREN, type, stored.data ? stored.data : paren_str);
    read_ch(lexer);
    return token;
}

token_t handle_string(lexer_t* lexer)
{
    char quote_char = lexer->ch;
    enum category_delimiter opening_delim_type = (quote_char == '"') ? DELIM_DQUOTE : DELIM_SQUOTE;
    read_ch(lexer);

    string_t str = read_string(lexer, quote_char);
    if(!str.data){
        add_report(lexer->reports, SEV_ERR, ERR_INVAL_STR, lexer->loc, DEFAULT_LEN, lexer->input->data);
        return new_token(CAT_SERVICE, SERV_ILLEGAL, "INVALID_STRING");
    }
    if(lexer->ch != quote_char){
        add_report(lexer->reports, SEV_ERR, ERR_UNCLO_STR, lexer->loc, DEFAULT_LEN, lexer->input->data);
        return new_token(CAT_SERVICE, SERV_ILLEGAL, "UNCLOSED_STRING");
    }
    if(opening_delim_type == DELIM_SQUOTE && str.length > 2){
        size_t len = 0;
        while(str.data[len++] != '\'');
        add_report(lexer->reports, SEV_ERR, ERR_INVAL_STR, lexer->loc, len, lexer->input->data);
        return new_token(CAT_SERVICE, SERV_ILLEGAL, "INVALID_STRING");
    }

    token_t string_token = new_token(CAT_LITERAL, LIT_STRING, str.data);
    read_ch(lexer);

    return string_token;
}

void handle_comment(lexer_t* lexer)
{
    if(!lexer) return;
    switch(peek_ch(lexer)){
        case '#': read_ch(lexer); read_ch(lexer); while(lexer->ch != '\n' && lexer->ch != '\0') read_ch(lexer); break;
        case '[': while(lexer->ch != ']' && peek_ch(lexer) != '#') read_ch(lexer); break;
        default:  while(lexer->ch != '\n') read_ch(lexer);
    }
}

string_t read_ident(lexer_t* lexer)
{
    char stack_buffer[IDENT_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    if(isdigit(lexer->ch)){
        add_report(lexer->reports, SEV_ERR, ERR_INVAL_IDENT, lexer->loc, DEFAULT_LEN, lexer->input->data);
        return (string_t){0};
    }

    while(isalnum(lexer->ch) || lexer->ch == '_'){
        if(length >= MAX_IDENT_SIZE){
            if(buffer != stack_buffer) free(buffer);
            add_report(lexer->reports, SEV_ERR, ERR_INVAL_IDENT, lexer->loc, length, lexer->input->data);
            return (string_t){0};
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
            if(!new_buf){
                if(buffer != stack_buffer) free(buffer);
                return (string_t){0};
            }
            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
            buffer = new_buf;
        }

        buffer[length++] = lexer->ch;
        read_ch(lexer);
    }

    buffer[length] = '\0';

    string_t stored = new_string(lexer->string_pool, buffer);
    if(!stored.data){
        if(buffer != stack_buffer) free(buffer);
        return (string_t){0};
    }

    if(buffer != stack_buffer) free(buffer);

    return stored;
}

string_t read_number(lexer_t* lexer, enum category_literal lit)
{
    if(!isdigit(lexer->ch)) return (string_t){0};

    char stack_buffer[NUM_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = NUM_SIZE;
    size_t length = 0;

    while(true){
        char ch = lexer->ch;
        if(ch == '\0') break;

        bool accept = false;
        switch(lit){
            case LIT_HEX: if(isxdigit(ch)) accept = true; break;
            case LIT_BIN: if(ch == '0' || ch == '1') accept = true; break;
            case LIT_OCT: if(ch >= '0' && ch <= '7') accept = true; break;
            default: break;
        }
        if(ch == '.' && lit != LIT_FLOAT){
            lit = LIT_FLOAT;
            accept = true;
        }
        else if(isdigit(ch)){
            accept = true;
        }

        if(!accept) break;

        if(length >= MAX_NUM_SIZE){
            if(buffer != stack_buffer) free(buffer);
            add_report(lexer->reports, SEV_ERR, ERR_INVAL_NUM, lexer->loc, length, lexer->input->data);
            return (string_t){0};
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
            if(!new_buf){
                if(buffer != stack_buffer) free(buffer);
                return (string_t){0};
            }
            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
            buffer = new_buf;
        }

        buffer[length++] = ch;
        read_ch(lexer);
    }

    if(length > 0 && (lit == LIT_HEX || lit == LIT_BIN || lit == LIT_OCT)){
        if(isalpha(lexer->ch) || lexer->ch == '_'){
            while(isalnum(lexer->ch) || lexer->ch == '_'){
                read_ch(lexer);
                length++;
            }

            add_report(lexer->reports, SEV_ERR, ERR_INVAL_LIT, (location_t){lexer->loc.line, lexer->loc.column - length}, length, lexer->input->data);

            if(buffer != stack_buffer) free(buffer);
            return (string_t){0};
        }
    }

    buffer[length] = '\0';

    string_t stored = new_string(lexer->string_pool, buffer);
    if(!stored.data){
        if(buffer != stack_buffer) free(buffer);
        return (string_t){0};
    }

    if(buffer != stack_buffer) free(buffer);

    return stored;
}

char read_escseq(lexer_t* lexer)
{
    if(!lexer) return '\0';

    char esc_seq = '\0';
    switch(lexer->ch){
        case 'n': esc_seq = '\n'; break;
        case 't': esc_seq = '\t'; break;
        case 'r': esc_seq = '\r'; break;
        case '"': esc_seq = '\"'; break;
        case '\\':esc_seq = '\\'; break;
        case '\'':esc_seq = '\''; break;
        case '0': esc_seq = '\0'; break;
        default:
            add_report(lexer->reports, SEV_WARN, ERR_INVAL_ESCSEQ, lexer->loc, DEFAULT_LEN, lexer->input->data);
            break;
    }
    read_ch(lexer);
    return esc_seq;
}

string_t read_string(lexer_t *lexer, char quote_char)
{
    char stack_buffer[STR_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = STR_SIZE;
    size_t length = 0;

    while(lexer->ch != quote_char && lexer->ch != '\0'){
        if(length >= MAX_STR_SIZE){
            if(buffer != stack_buffer) free(buffer);
            add_report(lexer->reports, SEV_ERR, ERR_INVAL_STR, lexer->loc, length, lexer->input->data);
            return (string_t){0};
        }

        if(lexer->ch == '\\'){
            read_ch(lexer);
            if(length >= capacity - 1){
                capacity *= 2;
                char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
                if(!new_buf){
                    if(buffer != stack_buffer) free(buffer);
                    return (string_t){0};
                }
                if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
                buffer = new_buf;
            }
            buffer[length++] = read_escseq(lexer);
            continue;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
            if(!new_buf){
                if(buffer != stack_buffer) free(buffer);
                return (string_t){0};
            }
            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
            buffer = new_buf;
        }

        buffer[length++] = lexer->ch;
        read_ch(lexer);
    }

    buffer[length] = '\0';

    string_t stored = new_string(lexer->string_pool, buffer);
    if(!stored.data){
        if(buffer != stack_buffer) free(buffer);
        return (string_t){0};
    }

    if(buffer != stack_buffer) free(buffer);
    return stored;
}
