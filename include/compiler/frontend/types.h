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

struct type {
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
            // bool is_variadic;
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

extern struct type* type_array;
extern struct type* type_function;
extern struct type* type_aggregate;

void init_types(void);
void free_types(void);
bool types_compatible(const struct type* a, const struct type* b);
struct type* datatype_to_type(enum category_datatype dt);
struct type* new_type_array(struct type* elem_type, size_t length);
struct type* new_type_function(struct type* return_type, struct type** param_types, const size_t param_count);
struct type* new_type_aggregate(struct symbol *scope, size_t member_count);
