#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core/lang/source.h"
#include "core/lang/filesystem.h"

static size_t count_lines(const char* src, size_t length)
{
    // TODO: implement
    return 0;
}

source_t* load_source_from_file(const char* filepath)
{
    // TODO: implement
    return NULL;
}

source_t* new_source(const char* filename)
{
    // TODO: implement
    return NULL;
}


void new_source_manager(void)
{
    // TODO: implement
}

void free_source_manager(source_manager_t* manager)
{
    // TODO: implement
    free_string_pool(manager->string_pool);
}

bool src_set_filename(source_t* src, const char* filename)
{
    // TODO: implement
    return true;
}

bool src_set_content(source_t* src, const char* content, size_t length)
{
    // TODO: implement
    return true;
}

int is_source_correct_extension(const char* filepath)
{
    // TODO: implement
    return 0;
}

size_t src_get_line(source_t* src, size_t offset)
{
    // TODO: implement
    return 0;
}

size_t src_get_column(source_t* src, size_t offset)
{
    // TODO: implement
    return 0;
}

bool src_get_line_range(source_t* src, size_t line, size_t* start, size_t* end)
{
    return true;
}

location_t new_location(void)
{
    return (location_t){1,1,0,0};
}

location_t loc_copy(location_t loc, size_t len)
{
    return (location_t){loc.line, loc.column + len, loc.offset, len};
}
