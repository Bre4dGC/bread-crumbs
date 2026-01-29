#pragma once

#include "core/strings.h"

#define SOURCE_EXTENSION ".brc"

typedef struct {
    string_t filename;
    string_t source;
} source_t;

source_t* load_source_from_file(const char* filepath);
void free_source(source_t* source);

int is_source_correct_extension(const char* filepath);
