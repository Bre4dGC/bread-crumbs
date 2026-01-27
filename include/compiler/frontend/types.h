#pragma once
#include <stddef.h>
#include <stdbool.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/frontend/symbol.h"

enum type_kind {
    // service types
    TYPE_UNKNOWN,
    TYPE_ERROR,

    // primitive types
    TYPE_VOID,
    TYPE_ANY,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_SHORT,
    TYPE_USHORT,
    TYPE_LONG,
    TYPE_ULONG,
    TYPE_FLOAT,
    TYPE_DECIMAL,
    TYPE_STR,
    TYPE_CHAR,

    // composite types
    TYPE_ARRAY,
    TYPE_FUNC,
    TYPE_STRUCT,
    TYPE_ENUM,
};

typedef struct type {
    enum type_kind kind;
    size_t size;
    size_t align;

    union {
        struct {
            struct type* elem_type;
            size_t length;
        } array;

        struct {
            struct type* return_type;
            struct type** param_types;
            size_t param_count;
        } func;

        // struct/enum
        struct {
            struct symbol* scope; // members are symbols in this scope
            size_t member_count;
        } compound;
    };
} type_t;

extern type_t* type_unknown;
extern type_t* type_error;
extern type_t* type_void;
extern type_t* type_any;
extern type_t* type_bool;
extern type_t* type_int;
extern type_t* type_uint;
extern type_t* type_short;
extern type_t* type_ushort;
extern type_t* type_long;
extern type_t* type_ulong;
extern type_t* type_float;
extern type_t* type_decimal;
extern type_t* type_str;
extern type_t* type_char;

void init_types(arena_t* arena);
void free_type(type_t* type);

type_t* new_type_array(arena_t* arena, type_t* elem_type, const size_t length);
type_t* new_type_function(arena_t* arena, type_t* return_type, type_t** param_types, const size_t param_count);
type_t* new_type_compound(arena_t* arena, struct symbol* scope, const size_t member_count);

bool types_compatible(const type_t* a, const type_t* b);
