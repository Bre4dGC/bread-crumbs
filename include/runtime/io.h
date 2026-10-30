#pragma once

#include <stdbool.h>    // bool
#include <stdint.h>     // int32_t

/*
 * Bread Crumbs runtime - console I/O.
 *
 * ABI notes:
 *   str      -> const char*  (UTF-8, NUL terminated)
 *   int      -> int32_t
 *   float    -> double
 *   decimal  -> double
 *   bool     -> bool
 *   char     -> char
 *
 * Strings returned by the runtime are allocated with malloc() and must be
 * released with brt_free().
 */

#ifdef __cplusplus
extern "C" {
#endif

/* stdout, newline terminated */
void brt_print(const char* s);
void brt_print_int(int32_t v);
void brt_print_float(double v);
void brt_print_bool(bool v);
void brt_print_char(char c);

/* stdout, no trailing newline */
void brt_write(const char* s);
void brt_write_int(int32_t v);
void brt_write_float(double v);
void brt_write_bool(bool v);
void brt_write_char(char c);

/* stderr, newline terminated */
void brt_eprint(const char* s);
void brt_eprint_int(int32_t v);
void brt_eprint_float(double v);
void brt_eprint_bool(bool v);
void brt_eprint_char(char c);

void brt_flush(void);

/* stdin; NULL on end-of-file, otherwise a malloc()'d NUL terminated string */
char* brt_read_line(void);
char* brt_read_all(void);

/* release a string returned by the runtime */
void brt_free(void* p);

#ifdef __cplusplus
}
#endif
