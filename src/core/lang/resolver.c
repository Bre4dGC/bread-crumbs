#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "core/ds/hashmap.h"
#include "core/lang/resolver.h"
#include "core/lang/source.h"
#include "core/lang/filesystem.h"

resolver_t* new_resolver(void)
{

}

void free_rs_sources(resolver_t* rs)
{
    // TODO: implement
}

bool rs_add_source(resolver_t* rs, const char* filename, const char* source)
{
    // TODO: implement
}

bool rs_add_source_file(resolver_t* rs, const char* filepath)
{
    // TODO: implement
}

bool rs_remove_source(resolver_t* rs, const char* filename)
{
    // TODO: implement
}

resolver_entry_t* rs_get_source(resolver_t* rs, const char* filename)
{
    // TODO: implement
}

bool rs_resolve(resolver_t* rs, const char* filename)
{
    // TODO: implement
}
