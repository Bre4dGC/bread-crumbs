#pragma once
#include <stddef.h>
#include <stdint.h>

#include "compiler/middle/ir.h"
#include "core/arena.h"
#include "core/hashmap.h"
#include "core/strings.h"

typedef struct object_t {
    enum {
        OBJECT_STRING,
        OBJECT_ARRAY,
        OBJECT_FUNCTION,
    } type;
    union {
        struct {
            char* value;
            size_t length;
        } string;
        struct {
            struct object_t** items;
            size_t length;
            size_t capacity;
        } array;
        struct {
            ir_t* ir;
            char* name;
        } function;
    } as;
} object_t;
