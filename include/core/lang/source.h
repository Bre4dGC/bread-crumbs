#pragma once

#include <stdbool.h>    // bool
#include <stddef.h>     // size_t

#include "core/ds/strings.h"    // string_pool_t

#define SOURCE_EXTENSION ".brc"

typedef struct {
    size_t line;
    size_t column;
} location_t;

typedef struct {
    string_t* filename;
    string_t* content;
    size_t line_count;
    bool loaded;
} source_t;

typedef struct {
    source_t* sources;
    source_t* current;
    string_pool_t* string_pool;
    size_t count;
} source_manager_t;

source_t* load_source_from_file(const char* filepath);
source_t* new_source(const char* filename);
void free_sources(source_manager_t* manager);

bool src_set_filename(source_t* source, const char* filename);
bool src_set_content(source_t* source, const char* content, size_t length);

bool src_append(source_t* source, const char* data, size_t length);
bool src_prepend(source_t* source, const char* data, size_t length);
bool src_insert(source_t* source, size_t index, const char* data, size_t length);
bool src_delete(source_t* source, size_t index, size_t length);
bool src_replace(source_t* source, size_t index, size_t length, const char* data, size_t new_length);

int is_source_correct_extension(const char* filepath);

size_t src_get_line(source_t* source, size_t offset);
size_t src_get_column(source_t* source, size_t offset);
bool src_get_line_range(source_t* source, size_t line, size_t* start, size_t* end);
