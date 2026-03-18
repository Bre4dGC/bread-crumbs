#pragma once

#include <stdbool.h>    // bool
#include <stddef.h>     // size_t

#include "core/ds/strings.h"    // string_pool_t
#include "core/ds/hashmap.h"    // hashmap_t
#include "core/lang/source.h"   // location_t

typedef struct {
    string_t filename;
    string_t source;
    bool loaded;
    bool resolved;
} resolver_entry_t;

typedef struct {
    hashmap_t sources;
    hashmap_t imports;
    size_t count;
} resolver_t;

resolver_t* new_resolver(void);
void free_rs_sources(resolver_t* rs);

bool rs_add_source(resolver_t* rs, const char* filename, const char* source);
bool rs_add_source_file(resolver_t* rs, const char* filepath);
bool rs_remove_source(resolver_t* rs, const char* filename);
resolver_entry_t* rs_get_source(resolver_t* rs, const char* filename);
bool rs_resolve(resolver_t* rs, const char* filename);
void free_rs_sources(resolver_t* rs);
