#include <stdio.h>
#include <stdlib.h>

#include "runtime/io.h"

/* -------------------------------------------------------------------------- */
/* output                                                                     */
/* -------------------------------------------------------------------------- */

static const char* safe_str(const char* s)
{
    return s ? s : "";
}

static void write_str(FILE* stream, const char* s)
{
    fputs(safe_str(s), stream);
}

static void write_line(FILE* stream, const char* s)
{
    fputs(safe_str(s), stream);
    fputc('\n', stream);
}

void brt_print(const char* s)
{
    write_line(stdout, s);
}

void brt_print_int(int32_t v)
{
    fprintf(stdout, "%d\n", (int)v);
}

void brt_print_float(double v)
{
    fprintf(stdout, "%g\n", v);
}

void brt_print_bool(bool v)
{
    write_line(stdout, v ? "true" : "false");
}

void brt_print_char(char c)
{
    fputc(c, stdout);
    fputc('\n', stdout);
}

void brt_write(const char* s)
{
    write_str(stdout, s);
}

void brt_write_int(int32_t v)
{
    fprintf(stdout, "%d", (int)v);
}

void brt_write_float(double v)
{
    fprintf(stdout, "%g", v);
}

void brt_write_bool(bool v)
{
    write_str(stdout, v ? "true" : "false");
}

void brt_write_char(char c)
{
    fputc(c, stdout);
}

void brt_eprint(const char* s)
{
    write_line(stderr, s);
}

void brt_eprint_int(int32_t v)
{
    fprintf(stderr, "%d\n", (int)v);
}

void brt_eprint_float(double v)
{
    fprintf(stderr, "%g\n", v);
}

void brt_eprint_bool(bool v)
{
    write_line(stderr, v ? "true" : "false");
}

void brt_eprint_char(char c)
{
    fputc(c, stderr);
    fputc('\n', stderr);
}

void brt_flush(void)
{
    fflush(stdout);
    fflush(stderr);
}

/* -------------------------------------------------------------------------- */
/* input                                                                      */
/* -------------------------------------------------------------------------- */

char* brt_read_line(void)
{
    size_t capacity = 128;
    size_t length = 0;

    char* buffer = malloc(capacity);
    if(!buffer) return NULL;

    int ch;
    while((ch = fgetc(stdin)) != EOF && ch != '\n'){
        if(length + 1 >= capacity){
            size_t new_capacity = capacity * 2;
            char* grown = realloc(buffer, new_capacity);
            if(!grown){
                free(buffer);
                return NULL;
            }
            buffer = grown;
            capacity = new_capacity;
        }
        buffer[length++] = (char)ch;
    }

    if(ch == EOF && length == 0){
        free(buffer);
        return NULL;
    }

    /* strip a trailing carriage return from CRLF input */
    if(length > 0 && buffer[length - 1] == '\r'){
        length--;
    }

    buffer[length] = '\0';
    return buffer;
}

char* brt_read_all(void)
{
    size_t capacity = 4096;
    size_t length = 0;

    char* buffer = malloc(capacity);
    if(!buffer) return NULL;

    while(true){
        if(length == capacity){
            size_t new_capacity = capacity * 2;
            char* grown = realloc(buffer, new_capacity);
            if(!grown){
                free(buffer);
                return NULL;
            }
            buffer = grown;
            capacity = new_capacity;
        }

        size_t read = fread(buffer + length, 1, capacity - length, stdin);
        length += read;

        if(read == 0){
            if(ferror(stdin)){
                free(buffer);
                return NULL;
            }
            break;
        }
    }

    /* make room for the terminator */
    if(length == capacity){
        char* grown = realloc(buffer, capacity + 1);
        if(!grown){
            free(buffer);
            return NULL;
        }
        buffer = grown;
    }

    buffer[length] = '\0';
    return buffer;
}

void brt_free(void* p)
{
    free(p);
}
