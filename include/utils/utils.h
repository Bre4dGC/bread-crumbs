#pragma once

#include <stdlib.h>
#include <string.h>

static inline char* strdup(const char* str) {
    char* dst = malloc(strlen (str) + 1);
    if (dst == NULL) return NULL;
    strcpy(dst, str);
    return dst;
}
