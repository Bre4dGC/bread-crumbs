#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/lang/source.h"
#include "core/lang/filesystem.h"

static size_t count_lines(const char* src, size_t length)
{
    if(!src) return 1;

    size_t lines = 1;
    for(size_t i = 0; i < length; ++i){
        if(src[i] == '\n') lines++;
    }
    return lines;
}

static string_t* make_string(const char* data, size_t length)
{
    string_t* str = malloc(sizeof(*str));
    if(!str) return NULL;

    char* copy = malloc(length + 1);
    if(!copy){
        free(str);
        return NULL;
    }

    if(length > 0 && data) memcpy(copy, data, length);
    copy[length] = '\0';

    str->data = copy;
    str->length = length;
    str->hash = 0;
    return str;
}

static void free_string(string_t* str)
{
    if(!str) return;
    free((void*)str->data);
    free(str);
}

source_t* load_source_from_file(const char* filepath)
{
    if(!filepath){
        source_t* src = calloc(1, sizeof(*src));
        if(!src) return NULL;

        src->content = make_string("", 0);
        if(!src->content){
            free(src);
            return NULL;
        }

        src->line_count = 1;
        src->loaded = true;
        return src;
    }

    FILE* f = fopen(filepath, "rb");
    if(!f) return NULL;

    if(fseek(f, 0, SEEK_END) != 0){
        fclose(f);
        return NULL;
    }

    long length = ftell(f);
    if(length < 0 || fseek(f, 0, SEEK_SET) != 0){
        fclose(f);
        return NULL;
    }

    char* content = malloc((size_t)length + 1);
    if(!content){
        fclose(f);
        return NULL;
    }

    size_t read = fread(content, 1, (size_t)length, f);
    fclose(f);

    if(read != (size_t)length){
        free(content);
        return NULL;
    }
    content[read] = '\0';

    source_t* src = calloc(1, sizeof(*src));
    if(!src){
        free(content);
        return NULL;
    }

    src->filename = make_string(filepath, strlen(filepath));
    src->content = malloc(sizeof(*src->content));
    if(!src->filename || !src->content){
        free(content);
        free_string(src->filename);
        free(src->content);
        free(src);
        return NULL;
    }

    src->content->data = content;
    src->content->length = read;
    src->content->hash = 0;
    src->line_count = count_lines(content, read);
    src->loaded = true;

    return src;
}

source_t* new_source(const char* filename)
{
    source_t* src = calloc(1, sizeof(*src));
    if(!src) return NULL;

    src->content = make_string("", 0);
    if(!src->content){
        free(src);
        return NULL;
    }

    src->line_count = 1;
    src->loaded = false;

    if(filename && !src_set_filename(src, filename)){
        free_source(src);
        return NULL;
    }

    return src;
}

void free_source(source_t* source)
{
    if(!source) return;

    free_string(source->filename);
    free_string(source->content);
    free(source);
}

source_manager_t* new_source_manager(string_pool_t* pool)
{
    source_manager_t* manager = calloc(1, sizeof(*manager));
    if(!manager) return NULL;

    manager->string_pool = pool;
    manager->capacity = 8;
    manager->sources = calloc(manager->capacity, sizeof(source_t*));
    if(!manager->sources){
        free(manager);
        return NULL;
    }

    manager->count = 0;
    manager->current = NULL;
    return manager;
}

source_t* source_manager_load(source_manager_t* manager, const char* filepath)
{
    if(!manager) return NULL;

    source_t* src = load_source_from_file(filepath);
    if(!src) return NULL;

    if(manager->count == manager->capacity){
        size_t new_capacity = manager->capacity ? manager->capacity * 2 : 8;
        source_t** resized = realloc(manager->sources, new_capacity * sizeof(source_t*));
        if(!resized){
            free_source(src);
            return NULL;
        }
        manager->sources = resized;
        manager->capacity = new_capacity;
    }

    manager->sources[manager->count++] = src;
    manager->current = src;
    return src;
}

void free_source_manager(source_manager_t* manager)
{
    if(!manager) return;

    for(size_t i = 0; i < manager->count; ++i){
        free_source(manager->sources[i]);
    }

    free(manager->sources);
    manager->sources = NULL;
    manager->current = NULL;
    manager->count = 0;
    manager->capacity = 0;
}

bool src_set_filename(source_t* src, const char* filename)
{
    if(!src || !filename) return false;

    string_t* name = make_string(filename, strlen(filename));
    if(!name) return false;

    free_string(src->filename);
    src->filename = name;
    return true;
}

bool src_set_content(source_t* src, const char* content, size_t length)
{
    if(!src || (!content && length > 0)) return false;

    string_t* str = make_string(content, length);
    if(!str) return false;

    free_string(src->content);
    src->content = str;
    src->line_count = count_lines(content, length);
    return true;
}

int is_source_correct_extension(const char* filepath)
{
    if(!filepath) return 0;

    const size_t path_len = strlen(filepath);
    const size_t ext_len = strlen(SOURCE_EXTENSION);
    if(path_len < ext_len) return 0;

    return strcmp(filepath + path_len - ext_len, SOURCE_EXTENSION) == 0;
}

size_t src_get_line(const source_t* src, size_t offset)
{
    if(!src || !src->content || !src->content->data) return 1;

    if(offset > src->content->length) offset = src->content->length;

    size_t line = 1;
    for(size_t i = 0; i < offset; ++i){
        if(src->content->data[i] == '\n') line++;
    }
    return line;
}

size_t src_get_column(const source_t* src, size_t offset)
{
    if(!src || !src->content || !src->content->data) return 1;

    if(offset > src->content->length) offset = src->content->length;

    size_t column = 1;
    for(size_t i = 0; i < offset; ++i){
        column = src->content->data[i] == '\n' ? 1 : column + 1;
    }
    return column;
}

bool src_get_line_range(const source_t* src, size_t line, size_t* start, size_t* end)
{
    if(!src || !src->content || !src->content->data || !start || !end || line == 0){
        return false;
    }

    const char* data = src->content->data;
    const size_t length = src->content->length;

    size_t pos = 0;
    for(size_t current = 1; current < line; ++current){
        const char* newline = memchr(data + pos, '\n', length - pos);
        if(!newline) return false;
        pos = (size_t)(newline - data) + 1;
    }

    const char* newline = memchr(data + pos, '\n', length - pos);
    size_t line_end = newline ? (size_t)(newline - data) : length;

    /* Do not include the carriage return of CRLF line endings. */
    if(line_end > pos && data[line_end - 1] == '\r') line_end--;

    *start = pos;
    *end = line_end;
    return true;
}

location_t new_location(void)
{
    return (location_t){1, 1, 0, 0};
}

location_t loc_copy(location_t loc, ptrdiff_t delta)
{
    if(delta >= 0){
        loc.column += (size_t)delta;
        loc.offset += (size_t)delta;
        loc.length = (size_t)delta;
        return loc;
    }

    const size_t back = (size_t)(-delta);
    loc.column = back < loc.column ? loc.column - back : 1;
    loc.offset = back < loc.offset ? loc.offset - back : 0;
    loc.length = back;
    return loc;
}
