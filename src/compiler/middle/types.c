#include <stdlib.h>
#include <stdalign.h>

#include "compiler/frontend/tokenizer.h"
#include "compiler/middle/types.h"
#include "compiler/diagnostic/errors.h"

struct type* type_unknown = NULL;
struct type* type_error = NULL;
struct type* type_void = NULL;
struct type* type_bool = NULL;
struct type* type_int = NULL;
struct type* type_uint = NULL;
struct type* type_float = NULL;
struct type* type_str = NULL;
struct type* type_char = NULL;
struct type* type_array = NULL;
struct type* type_function = NULL;
struct type* type_aggregate = NULL;

struct type* new_type(const enum type_kind kind, const size_t size, const size_t align);

void init_types(void)
{
    type_unknown = new_type(TYPE_UNKNOWN, 0, 1);
    type_error =   new_type(TYPE_ERROR,   0, 1);
    type_void =    new_type(TYPE_VOID,    0, 1);
    type_bool =    new_type(TYPE_BOOL,  sizeof(bool),  alignof(bool));
    type_int =     new_type(TYPE_INT,   sizeof(int),   alignof(int));
    type_uint =    new_type(TYPE_UINT,  sizeof(unsigned int), alignof(unsigned int));
    type_float =   new_type(TYPE_FLOAT, sizeof(float), alignof(float));
    type_str =     new_type(TYPE_STR,   sizeof(char*), alignof(char*));
    type_char =    new_type(TYPE_CHAR,  sizeof(char),  alignof(char));
}

struct type* new_type(const enum type_kind kind, const size_t size, const size_t align)
{
    struct type* type = (struct type*)malloc(sizeof(struct type));
    if(!type) return NULL;
    type->kind = kind;
    type->size = size;
    type->align = align;
    return type;
}

struct type* new_type_array(struct type* elem_type, const size_t length)
{
    struct type* type = (struct type*)malloc(sizeof(struct type));
    if(!type) return NULL;
    
    type->kind = TYPE_ARRAY;
    type->array.elem_type = elem_type;
    type->array.length = length;
    
    if(length > 0 && elem_type){
        type->size = elem_type->size * length;
        type->align = elem_type->align;
    }
    else {
        type->size = sizeof(void*); // pointer to dynamic array
        type->align = sizeof(void*);
    }
    
    return type;
}

struct type* new_type_function(struct type* return_type, struct type** param_types, const size_t param_count)
{
    struct type* type = (struct type*)malloc(sizeof(struct type));
    if(!type) return NULL;

    type->kind = TYPE_FUNCTION;
    type->func.return_type = return_type;
    type->func.param_types = param_types;
    type->func.param_count = param_count;
    // type->func.is_variadic = false;
    type->size = 8;  // function pointer size
    type->align = 8;

    return type;
}

struct type* new_type_aggregate(struct symbol* scope, const size_t member_count)
{
    struct type* type = (struct type*)malloc(sizeof(struct type));
    if(!type) return NULL;
    
    type->kind = TYPE_STRUCT;
    type->aggregate.scope = scope;
    type->aggregate.member_count = member_count;
    type->size = 0;
    type->align = 1;
    
    return type;
}

bool types_equal(const struct type* a, const struct type* b)
{
    if(!a || !b) return a == b;
    if(a->kind != b->kind) return false;
    
    switch(a->kind){
        case TYPE_VOID:  case TYPE_BOOL:
        case TYPE_INT:   case TYPE_UINT:
        case TYPE_FLOAT: case TYPE_STR:
        case TYPE_CHAR:  case TYPE_UNKNOWN:
        case TYPE_ERROR:
            return true;  // primitive types are equal if kinds match
            
        case TYPE_ARRAY:
            return a->array.length == b->array.length && types_equal(a->array.elem_type, b->array.elem_type);
                   
        case TYPE_FUNCTION:
            if(!types_equal(a->func.return_type, b->func.return_type)){
                return false;
            }
            if(a->func.param_count != b->func.param_count){
                return false;
            }
            for(size_t i = 0; i < a->func.param_count; i++){
                if(!types_equal(a->func.param_types[i], b->func.param_types[i])){
                    return false;
                }
            }
            return true;
            
        default: return a == b; // for struct/enum/union, compare by identity
    }
}

bool types_compatible(const struct type* a, const struct type* b)
{
    // compatible is more relaxed than equal
    if(types_equal(a, b)) return true;
    
    // int and uint are compatible for some operations
    if((a->kind == TYPE_INT && b->kind == TYPE_UINT) || (a->kind == TYPE_UINT && b->kind == TYPE_INT)){
        return true;
    }
    
    // unknown type is compatible with anything (for type inference)
    if(a->kind == TYPE_UNKNOWN || b->kind == TYPE_UNKNOWN){
        return true;
    }
    
    return false;
}

bool is_numeric_type(const struct type* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_FLOAT;
}

bool is_integer_type(const struct type* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT || type->kind == TYPE_UINT;
}

bool is_signed_type(const struct type* type)
{
    if(!type) return false; 
    return type->kind == TYPE_INT || type->kind == TYPE_FLOAT;
}

struct type* datatype_to_type(enum category_datatype dt)
{
    switch(dt){
        case DT_VOID:  return type_void;
        case DT_BOOL:  return type_bool;
        case DT_INT:   return type_int;
        case DT_UINT:  return type_uint;
        case DT_FLOAT: return type_float;
        case DT_STR:   return type_str;
        default:       return type_error;
    }
}

void free_types(void)
{
    free(type_unknown);
    free(type_error);
    free(type_void);
    free(type_bool);
    free(type_char);
    free(type_int);
    free(type_uint);
    free(type_float);
    free(type_str);

    type_unknown = NULL;
    type_error = NULL;
    type_void = NULL;
    type_bool = NULL;
    type_char = NULL;
    type_int = NULL;
    type_uint = NULL;
    type_float = NULL;
    type_str = NULL;
}
