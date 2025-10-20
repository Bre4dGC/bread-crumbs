#pragma once
#include <stddef.h>
#include <stdbool.h>

#include "symbol.h"

enum type_kind {
    // service types
    TYPE_UNKNOWN,
    TYPE_ERROR,
    
    // primitive types
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_STR,
    TYPE_CHAR,
    
    // composite types
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_UNION,
};

struct type {
    enum type_kind kind;
    size_t size;
    size_t alignment;

    union {
        struct {
            struct type* elem_type;
            size_t length;
        } array;
        
        struct {
            struct type* return_type;
            struct type** param_types;
            size_t param_count;
            bool is_variadic;
        } func;
        
        // struct/union
        struct {
            struct symbol* scope;
            size_t member_count;
        } aggregate;        
    };
};

extern struct type* type_unknown;
extern struct type* type_error;
extern struct type* type_void;
extern struct type* type_bool;
extern struct type* type_int;
extern struct type* type_uint;
extern struct type* type_float;
extern struct type* type_str;
extern struct type* type_char;
