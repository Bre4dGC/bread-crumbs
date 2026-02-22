#include "compiler/middle/builder.h"

builder_t* new_builder(arena_t* arena)
{
    builder_t* builder = arena_alloc(arena, sizeof(builder_t), alignof(builder_t));
    if(!builder) return NULL;
    builder->ir = new_ir(arena);
    if(!builder->ir) return NULL;
    return builder;
}

ir_t* build_ir(builder_t* builder)
{
    // TODO: implement
}
