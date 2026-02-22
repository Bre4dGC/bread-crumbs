#pragma once
#include <stddef.h>
#include <stdint.h>

#include "compiler/middle/ir.h"

enum obj_type {
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_FUNCTION,
};

typedef struct object {
    enum obj_type type;
    union {
        struct {
            char* value;
            size_t length;
        } string;
        struct {
            struct object** items;
            size_t length;
            size_t capacity;
        } array;
        struct {
            ir_t* ir;
            char* name;
        } function;
    } as;
} object_t;
