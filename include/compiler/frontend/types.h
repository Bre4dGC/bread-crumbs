#pragma once
#include <stddef.h>
#include <stdbool.h>

#include "compiler/frontend/symbol.h"

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
        
        // struct/unions
        struct {
            struct symbol* scope; // members are symbols in this scope
            size_t member_count;
        } compound;
    };
} type_t;

extern type_t* type_unknown;
extern type_t* type_error;
extern type_t* type_void;
extern type_t* type_bool;
extern type_t* type_int;
extern type_t* type_uint;
extern type_t* type_float;
extern type_t* type_str;
extern type_t* type_char;

void init_types(void);
void free_type(type_t* type);
void free_types(void);

type_t* new_type(const enum type_kind kind, const size_t size, const size_t align);
type_t* new_type_array(type_t* elem_type, const size_t length);
type_t* new_type_function(type_t* return_type, type_t** param_types, const size_t param_count);
type_t* new_type_compound(struct symbol* scope, const size_t member_count);

bool types_compatible(const type_t* a, const type_t* b);
type_t* datatype_to_type(const enum category_datatype dt);
