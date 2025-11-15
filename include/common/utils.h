#pragma once

#include <stdlib.h>
#include <string.h>

static inline char* util_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, str, len);
    return copy;
}
