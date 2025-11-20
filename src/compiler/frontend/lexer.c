#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include "compiler/frontend/lexer.h"

#define IDENT_SIZE 16
#define NUM_SIZE 32
#define STR_SIZE 64

#define MAX_IDENT_SIZE 64
#define MAX_NUM_SIZE 128
#define MAX_STR_SIZE 4096

size_t begining_line_pos = 0;

token_t handle_operator(lexer_t* lex);
token_t handle_parentesis(lexer_t* lex);
token_t handle_number(lexer_t* lex);
token_t handle_identifier(lexer_t* lex);
token_t handle_string(lexer_t* lex);
void handle_commment(lexer_t* lex);

char* read_ident(lexer_t* lex);
char* read_num(lexer_t* lex);
char* read_str(lexer_t* lex, char quote_char);
char read_esc_seq(lexer_t* lex);

void read_ch(lexer_t *lex)
{
    if (!lex || !lex->input) return;

    lex->pos = lex->nextpos++;
    if (lex->pos >= lex->input_len) {
        lex->ch = 0;
    } else {
        lex->ch = lex->input[lex->pos];
        lex->column++;
    }
}

void skip_whitespace(lexer_t *lex)
{
    while (1){
        switch (lex->ch){
            case ' ': case '\t': case '\r':
                read_ch(lex);
                break;

            case '\n':
                read_ch(lex);
                lex->line++;
                lex->column = 1;
                begining_line_pos = lex->pos;
                break;

            default: return;
        }
    }
}

char peek_ch(const lexer_t* lex)
{
    if(!lex || !lex->input) return '\0';

    if(lex->nextpos < lex->input_len){
        return lex->input[lex->nextpos];
    }
    return '\0';
}

lexer_t* new_lexer(const char* input)
{
    if(!input) return NULL;

    lexer_t* lexer = (lexer_t*)malloc(sizeof(lexer_t));
    if(!lexer) return NULL;

    lexer->input_len = strlen(input);
    lexer->input = (char*)malloc((lexer->input_len + 1) * sizeof(char));
    if(!lexer->input){
        free(lexer);
        return NULL;
    }

    strcpy(lexer->input, input);
    lexer->input[lexer->input_len] = '\0';

    lexer->ch = lexer->input_len > 0 ? input[0] : '\0';

    lexer->pos = 0;
    lexer->nextpos = lexer->pos + 1;
    lexer->line = 1;
    lexer->column = 1;

    lexer->paren_balance = 0;

    lexer->errors = NULL;
    lexer->errors_count = 0;

    return lexer;
}

void new_lexer_error(lexer_t* lex, report_t* err)
{
    if(!lex || !err){
        if(err) free_report(err);
        return;
    }

    if(!lex->errors){
        lex->errors = (report_t**)malloc(sizeof(report_t*));
        if(!lex->errors){
            free_report(err);
            return;
        }
        lex->errors[0] = err;
        lex->errors_count = 1;
        return;
    }

    report_t** new_errors = (report_t**)realloc(
        lex->errors,
        (lex->errors_count + 1) * sizeof(report_t*)
    );
    if(!new_errors){
        free_report(err);
        return;
    }

    lex->errors = new_errors;
    lex->errors[lex->errors_count] = err;
    lex->errors_count++;
}


token_t next_token(lexer_t* lex)
{
    if(!lex) return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "NULL_LEXER");

    while(1){
        skip_whitespace(lex);

        if(lex->ch == '#'){
            handle_commment(lex);
            continue;
        }
        break;
    }

    const char ch_str[2] ={lex->ch, '\0'};
    token_t token;

    switch (lex->ch){
        case '+': case '-':
        case '*': case '/':
        case '%': case '=':
        case '!': case '<':
        case '>': case '&':
        case '|': case '.':
        case ',': case ':':
        case ';': case '?':
            token = handle_operator(lex);
            break;

        case '(': case ')':
        case '{': case '}':
        case '[': case ']':
            token = handle_parentesis(lex);
            break;

        case '"': case '\'':
            token = handle_string(lex);
            break;

        case '\0':
            if(lex->paren_balance != 0){
                report_t* err = new_report(
                    SEVERITY_ERROR, ERROR_UNMATCHED_PAREN, lex->line, lex->column, 1, lex->input
                );
                new_lexer_error(lex, err);
            }
            token = new_token(CATEGORY_SERVICE, SERV_EOF, "EOF");
            break;

        default:
            if(isalnum(lex->ch) || lex->ch == '_'){
                token = handle_identifier(lex);
            }
            else{
                int length = 0;
                while(lex->ch != '\0' && !isalnum(lex->ch) && !isspace(lex->ch)){
                    read_ch(lex);
                    length++;
                }
                token = new_token(CATEGORY_SERVICE, SERV_ILLEGAL, ch_str);
                report_t* err = new_report(
                    SEVERITY_ERROR, ERROR_ILLEGAL_CHARACTER, lex->line, lex->column - length, length, lex->input
                );
                new_lexer_error(lex, err);
                read_ch(lex);
            }
            break;
    }
    return token;
}

const keyword_t* find_oper(const char* op)
{
    for (size_t i = 0; i < operators_count; i++){
        if(strcmp(op, operators[i].literal) == 0) return &operators[i];
    }
    return NULL;
}

token_t handle_operator(lexer_t* lex)
{
    const char current = lex->ch;
    const char next = peek_ch(lex);

    if(next != '\0'){
        const char potential_op[3] ={current, next, '\0'};
        const keyword_t *op = find_oper(potential_op);
        if(op){
            read_ch(lex);
            read_ch(lex);
            return new_token(CATEGORY_OPERATOR, op->type, op->literal);
        }
    }

    read_ch(lex);
    const char oper[2] ={current, '\0'};
    switch (current){
        case '+': return new_token(CATEGORY_OPERATOR, OPER_PLUS,     oper);
        case '-': return new_token(CATEGORY_OPERATOR, OPER_MINUS,    oper);
        case '*': return new_token(CATEGORY_OPERATOR, OPER_ASTERISK, oper);
        case '/': return new_token(CATEGORY_OPERATOR, OPER_SLASH,    oper);
        case '%': return new_token(CATEGORY_OPERATOR, OPER_PERCENT,  oper);
        case '=': return new_token(CATEGORY_OPERATOR, OPER_ASSIGN,   oper);
        case '!': return new_token(CATEGORY_OPERATOR, OPER_NOT,      oper);
        case '<': return new_token(CATEGORY_OPERATOR, OPER_LANGLE,   oper);
        case '>': return new_token(CATEGORY_OPERATOR, OPER_RANGLE,   oper);
        case '.': return new_token(CATEGORY_OPERATOR, OPER_DOT,      oper);
        case ',': return new_token(CATEGORY_OPERATOR, OPER_COMMA,    oper);
        case ':': return new_token(CATEGORY_OPERATOR, OPER_COLON,    oper);
        case ';': return new_token(CATEGORY_OPERATOR, OPER_SEMICOLON,oper);
        case '?': return new_token(CATEGORY_OPERATOR, OPER_QUESTION, oper);
        default:  return new_token(CATEGORY_SERVICE,  SERV_ILLEGAL,  oper);
    }
}

const keyword_t* find_keyword(const char* ident)
{
    for (size_t i = 0; i < keywords_count; ++i){
        if(strcmp(ident, keywords[i].literal) == 0){
            return &keywords[i];
        }
    }
    return NULL;
}

token_t handle_identifier(lexer_t* lex)
{
    if(!lex) return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "NULL_LEXER");

    token_t tok;

    if(isdigit(lex->ch)){
        tok = handle_number(lex);
        return tok;
    }

    char *ident = read_ident(lex);
    if(!ident){
        while(isalnum(lex->ch) || lex->ch == '_') read_ch(lex); // skip remaining alphanumeric characters to recover
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "INVALID_IDENT");
    }

    const keyword_t *kw = find_keyword(ident);

    if(kw) tok = new_token(kw->category, kw->type, ident);
    else if(strcmp(ident, "true")  == 0) tok = new_token(CATEGORY_LITERAL, LIT_TRUE, ident);
    else if(strcmp(ident, "false") == 0) tok = new_token(CATEGORY_LITERAL, LIT_FALSE, ident);
    else if(strcmp(ident, "null")  == 0) tok = new_token(CATEGORY_LITERAL, LIT_NULL, ident);
    else tok = new_token(CATEGORY_LITERAL, LIT_IDENT, ident);

    free(ident);
    return tok;
}

token_t handle_number(lexer_t *lex)
{
    char* num_str = read_num(lex);
    if(!num_str) return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "BAD_NUMBER");

    bool is_float = strchr(num_str, '.') != NULL;
    bool is_hex   = strstr(num_str, "0x") == num_str;
    bool is_bin   = strstr(num_str, "0b") == num_str;

    token_t token = new_token(CATEGORY_LITERAL, is_float ? LIT_FLOAT : LIT_NUMBER, num_str);
    if(is_hex) token.type_literal = LIT_HEX;
    else if(is_bin) token.type_literal = LIT_BIN;

    free(num_str);
    return token;
}

token_t handle_parentesis(lexer_t *lex)
{
    if(lex->ch == '(' || lex->ch == '{' || lex->ch == '['){
        lex->paren_balance++;
    }
    else if(lex->ch == ')' || lex->ch == '}' || lex->ch == ']'){
        if(lex->paren_balance == 0){
            report_t* err = new_report(
                SEVERITY_ERROR, ERROR_UNMATCHED_PAREN, lex->line, lex->column, 1, lex->input
            );
            new_lexer_error(lex, err);
        }
        else{
            lex->paren_balance--;
        }
    }

    enum category_paren type = 0;
    switch (lex->ch){
        case '(': type = PAR_LPAREN; break;
        case ')': type = PAR_RPAREN; break;
        case '{': type = PAR_LBRACE; break;
        case '}': type = PAR_RBRACE; break;
        case '[': type = PAR_LBRACKET; break;
        case ']': type = PAR_RBRACKET; break;
    }
    const token_t token = new_token(CATEGORY_PAREN, type, (char[]){lex->ch, '\0'});
    read_ch(lex);
    return token;
}

token_t handle_string(lexer_t* lex)
{
    char quote_char = lex->ch;
    enum category_delimiter opening_delim_type = (quote_char == '"') ? DELIM_QUOTE : DELIM_SQUOTE;
    token_t opening_delim = new_token(CATEGORY_DELIMITER, opening_delim_type, (char[]){quote_char, '\0'});
    free_token(&opening_delim);
    read_ch(lex);

    char* str = read_str(lex, quote_char);
    if(!str){
        report_t* err = new_report(
            SEVERITY_ERROR, ERROR_INVALID_STRING, lex->line, lex->column, 1, lex->input
        );
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "INVALID_STRING");
    }
    if(lex->ch != quote_char){
        free(str);
        report_t* err = new_report(
            SEVERITY_ERROR, ERROR_UNCLOSED_STRING, lex->line, lex->column+1, 1, lex->input
        );
        new_lexer_error(lex, err);
        return new_token(CATEGORY_SERVICE, SERV_ILLEGAL, "UNCLOSED_STRING");
    }

    token_t string_token = new_token(CATEGORY_LITERAL, LIT_STRING, str);
    free(str);

    token_t closing_delim = new_token(CATEGORY_DELIMITER, opening_delim_type, (char[]){lex->ch, '\0'});
    free_token(&closing_delim);
    read_ch(lex);

    return string_token;
}

void handle_commment(lexer_t* lex)
{
    if(!lex) return;
    while (lex->ch != '\n' && lex->ch != '\0') read_ch(lex); // read until end
}

char* read_ident(lexer_t* lex)
{
    char stack_buffer[IDENT_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = IDENT_SIZE;
    size_t length = 0;

    if(isdigit(lex->ch)){
        report_t* err = new_report(
            SEVERITY_ERROR, ERROR_INVALID_IDENTIFIER, lex->line, lex->column, 1, lex->input
        );
        new_lexer_error(lex, err);
        return NULL;
    }

    while(isalnum(lex->ch) || lex->ch == '_'){
        if(length >= MAX_IDENT_SIZE){
            if(buffer != stack_buffer) free(buffer);
            report_t* err = new_report(
                SEVERITY_ERROR, ERROR_INVALID_IDENTIFIER, lex->line, lex->column, length, lex->input
            );
            new_lexer_error(lex, err);
            return NULL;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer == stack_buffer ? NULL : buffer, capacity * sizeof(char));
            if(!new_buf){
                if(buffer != stack_buffer) free(buffer);
                return NULL;
            }

            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));

            buffer = new_buf;
        }

        buffer[length++] = lex->ch;
        read_ch(lex);
    }

    buffer[length] = '\0';

    if(buffer == stack_buffer){
        char* heap_copy = (char*)malloc((length + 1) * sizeof(char));
        if(!heap_copy) return NULL;

        strcpy(heap_copy, buffer);
        return heap_copy;
    }
    return buffer;
}

char* read_num(lexer_t* lex)
{
    if(!isdigit(lex->ch)) return NULL;

    char stack_buffer[NUM_SIZE];
    char* buffer = stack_buffer;
    size_t capacity = NUM_SIZE;
    size_t length = 0;

    bool is_hex = false;
    bool is_bin = false;
    bool is_float = false;

    if(lex->ch == '0'){
        char next = peek_ch(lex);
        if(next == 'x'){
            buffer[length++] = lex->ch; read_ch(lex);
            buffer[length++] = lex->ch; read_ch(lex);
            is_hex = true;
        }
        else if(next == 'b'){
            buffer[length++] = lex->ch; read_ch(lex);
            buffer[length++] = lex->ch; read_ch(lex);
            is_bin = true;
        }
    }

    while (1){
        char ch = lex->ch;
        if(ch == '\0') break;

        bool accept = false;
        if(is_hex){
            if(isxdigit(ch)) accept = true;
        }
        else if(is_bin){
            if(ch == '0' || ch == '1') accept = true;
        }
        else{
            if(ch == '.' && !is_float){
                is_float = true;
                accept = true;
            }
            else if(isdigit(ch)){
                accept = true;
            }
        }

        if(!accept) break;

        if(length >= MAX_NUM_SIZE){
            if(buffer != stack_buffer) free(buffer);
            report_t* err = new_report(
                SEVERITY_ERROR, ERROR_INVALID_NUMBER, lex->line, lex->column, length, lex->input
            );
            new_lexer_error(lex, err);
            return NULL;
        }

        if(length >= capacity - 1){
            size_t new_capacity = capacity * 2;
            char* new_buf = (char*)malloc(new_capacity * sizeof(char));
            if(!new_buf){
                if(buffer != stack_buffer) free(buffer);
                return NULL;
            }
            if(buffer == stack_buffer) memcpy(new_buf, stack_buffer, length * sizeof(char));
            else memcpy(new_buf, buffer, length * sizeof(char));
            if(buffer != stack_buffer) free(buffer);
            buffer = new_buf;
            capacity = new_capacity;
        }

        buffer[length++] = ch;
        read_ch(lex);
    }

    if(length > 0 && !is_hex && !is_bin){
        if(isalpha(lex->ch) || lex->ch == '_'){
            while (isalnum(lex->ch) || lex->ch == '_'){
                read_ch(lex);
                length++;
            }
            report_t* err = new_report(
                SEVERITY_ERROR, ERROR_INVALID_LITERAL, lex->line, lex->column - length, length, lex->input
            );
            new_lexer_error(lex, err);
            if(buffer != stack_buffer) free(buffer);
            return NULL;
        }
    }

    buffer[length] = '\0';

    if(buffer == stack_buffer){
        char* heap_copy = (char*)malloc((length + 1) * sizeof(char));
        if(!heap_copy) return NULL;
        memcpy(heap_copy, buffer, length + 1);
        return heap_copy;
    }

    if(length < capacity / 2){
        char* shrunk_buf = (char*)realloc(buffer, (length + 1) * sizeof(char));
        if(shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

char read_esc_seq(lexer_t* lex)
{
    if(!lex) return '\0';

    char esc_seq = '\0';
    switch (lex->ch){
        case 'n': esc_seq = '\n'; break;
        case 't': esc_seq = '\t'; break;
        case 'r': esc_seq = '\r'; break;
        case '"': esc_seq = '\"'; break;
        case '\\':esc_seq = '\\'; break;
        case '\'':esc_seq = '\''; break;
        case '0': esc_seq = '\0'; break;
        default:
            esc_seq = lex->ch;
            report_t* err = new_report(
                SEVERITY_WARNING, ERROR_INVALID_ESCAPE_SEQUENCE, lex->line, lex->column, 1, lex->input
            );
            new_lexer_error(lex, err);
            break;
    }
    read_ch(lex);
    return esc_seq;
}

char* read_str(lexer_t *lex, char quote_char)
{
    size_t capacity = STR_SIZE;
    size_t length = 0;
    char* buffer = (char*)malloc(capacity * sizeof(char));
    if(!buffer) return NULL;

    while (lex->ch != quote_char && lex->ch != '\0'){
        if(length >= MAX_STR_SIZE){
            free(buffer);
            report_t* err = new_report(
                SEVERITY_ERROR, ERROR_INVALID_STRING, lex->line, lex->column, length, lex->input
            );
            new_lexer_error(lex, err);
            return NULL;
        }

        if(lex->ch == '\\'){
            read_ch(lex);
            buffer[length++] = read_esc_seq(lex);
            continue;
        }

        if(length >= capacity - 1){
            capacity *= 2;
            char* new_buf = (char*)realloc(buffer, capacity * sizeof(char));
            if(!new_buf){
                free(buffer);
                return NULL;
            }
            buffer = new_buf;
        }

        buffer[length++] = lex->ch;
        read_ch(lex);
    }

    buffer[length] = '\0';

    if(length < capacity / 2){
        char *shrunk_buf = (char*)realloc(buffer, (length + 1) * sizeof(char));
        if(shrunk_buf) buffer = shrunk_buf;
    }

    return buffer;
}

void free_lexer(lexer_t* lex)
{
    if(!lex) return;
    if(lex->errors){
        for(size_t i = 0; i < lex->errors_count; i++){
            if(lex->errors[i]) free_report(lex->errors[i]);
        }
        free(lex->errors);
    }
    free(lex->input);

    lex->input = NULL;
    lex->errors = NULL;

    free(lex);
    lex = NULL;
}
