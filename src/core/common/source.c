#pragma once
#include <stddef.h>
#include <string.h>

#include "core/strings.h"
#include "core/common/filesystem.h"

source_t* new_source(string_t filename);
void free_source(source_t* source);

string_t source_get_filename(source_t* source);
string_t source_get_source(source_t* source);

void source_set_filename(source_t* source, string_t filename);
void source_set_source(source_t* source, string_t source);

void source_append(source_t* source, string_t data);
void source_prepend(source_t* source, string_t data);

void source_replace(source_t* source, string_t data, string_t replacement);
void source_insert(source_t* source, string_t data, size_t index);
void source_delete(source_t* source, size_t index);

int is_source_correct_extension(const char* filepath)
{
    size_t len = strlen(filepath);
    if(len < 4) return 0;

    return strcmp(filepath + len - 4, SOURCE_EXTENSION) == 0;
}
