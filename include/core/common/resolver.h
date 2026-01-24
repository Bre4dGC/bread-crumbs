#pragma once

#include "core/strings.h"
#include "core/hashmap.h"

typedef struct {
    hashmap_t sources;
} resolver_t;

resolver_t* new_resolver(void);
void free_resolver(resolver_t* resolver);

void rslv_resolve(resolver_t* resolver, string_t filename);
void rslv_add_src(resolver_t* resolver, string_t filename, string_t src);
void rslv_remove_src(resolver_t* resolver, string_t filename);
void rslv_get_src(resolver_t* resolver, string_t filename);
void rslv_set_src(resolver_t* resolver, string_t filename, string_t src);
void rslv_free_src(resolver_t* resolver, string_t filename);
void rslv_free_all_srcs(resolver_t* resolver);