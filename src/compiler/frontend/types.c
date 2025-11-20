#include <stdlib.h>
#include <stdalign.h>

#include "compiler/core/diagnostic.h"
#include "compiler/frontend/tokenizer.h"
#include "compiler/frontend/types.h"

type_t* type_unknown = NULL;
type_t* type_error = NULL;
type_t* type_void = NULL;
type_t* type_bool = NULL;
type_t* type_int = NULL;
type_t* type_uint = NULL;
type_t* type_float = NULL;
type_t* type_str = NULL;
type_t* type_char = NULL;

type_t* new_type(const enum type_kind kind, const size_t size, const size_t align)
{
    type_t* type = (type_t*)malloc(sizeof(type_t));
    if(!type) return NULL;

    type->kind = kind;
    type->size = size;
    type->align = align;

    return type;
}

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

type_t* new_type_array(type_t* elem_type, const size_t length)
{
    type_t* array = new_type(TYPE_ARRAY, 0, 1);
    if(!array) return NULL;

    array->array.elem_type = elem_type;
    array->array.length = length;
    
    if(length > 0 && elem_type){
        array->size = elem_type->size * length;
        array->align = elem_type->align;
    }
    else {
        array->size = sizeof(void*);
        array->align = sizeof(void*);
    }
    
    return array;
}

type_t* new_type_function(type_t* return_type, type_t** param_types, const size_t param_count)
{
    type_t* function = new_type(TYPE_FUNCTION, 0, 1);
    if(!function) return NULL;

    function->func.return_type = return_type;
    function->func.param_types = param_types;
    function->func.param_count = param_count;

    if(return_type){
        function->size = return_type->size;
        function->align = return_type->align;
    }
    else {
        function->size = sizeof(void*);
        function->align = sizeof(void*);
    }
    
    return function;
}

type_t* new_type_compound(struct symbol* scope, const size_t member_count)
{
    type_t* compound = new_type(TYPE_STRUCT, 0, 1);
    if(!compound) return NULL;

    compound->compound.scope = scope;
    compound->compound.member_count = member_count;

    return compound;
}

bool types_equal(const type_t* a, const type_t* b)
{
    if(!a || !b) return false;
    if(a->kind != b->kind) return false;

    switch(a->kind){
        case TYPE_ARRAY:
            return types_equal(a->array.elem_type, b->array.elem_type) && (a->array.length == b->array.length);

        case TYPE_FUNCTION:
            if(!types_equal(a->func.return_type, b->func.return_type)) return false;
            if(a->func.param_count != b->func.param_count) return false;
            for(size_t i = 0; i < a->func.param_count; ++i){
                if(!types_equal(a->func.param_types[i], b->func.param_types[i])){
                    return false;
                }
            }
            return true;

        case TYPE_STRUCT: case TYPE_UNION: 
            return a->compound.member_count == b->compound.member_count;

        default: return true;
    }
}

bool types_compatible(const type_t* a, const type_t* b)
{
    if(!a || !b) return false;

    if(types_equal(a, b)) return true;
    
    // int and uint are compatible
    if((a->kind == TYPE_INT && b->kind == TYPE_UINT) || (a->kind == TYPE_UINT && b->kind == TYPE_INT)){
        return true;
    }
    
    // unknown type is compatible with any type
    if(a->kind == TYPE_UNKNOWN || b->kind == TYPE_UNKNOWN){
        return true;
    }
    
    return false;
}

bool is_numeric_type(const type_t* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_FLOAT;
}

bool is_integer_type(const type_t* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT || type->kind == TYPE_UINT;
}

bool is_signed_type(const type_t* type)
{
    if(!type) return false; 
    return type->kind == TYPE_INT || type->kind == TYPE_FLOAT;
}

type_t* datatype_to_type(enum category_datatype dt)
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

void free_type(type_t* type)
{
    if(!type) return;
    
    switch(type->kind){
        case TYPE_ARRAY:
            if(type->array.elem_type){
                free_type(type->array.elem_type);
                type->array.elem_type = NULL;
            }
            break;

        case TYPE_FUNCTION:
            if(type->func.return_type){
                free_type(type->func.return_type);
                type->func.return_type = NULL;
            }
            if(type->func.param_types){
                for(size_t i = 0; i < type->func.param_count; ++i){
                    if(type->func.param_types[i]){
                        free_type(type->func.param_types[i]);
                        type->func.param_types[i] = NULL;
                    }
                }
                free(type->func.param_types);
                type->func.param_types = NULL;
            }
            break;

        case TYPE_STRUCT:
        case TYPE_UNION:
            if(type->compound.scope){
                free_scope(type->compound.scope);
                type->compound.scope = NULL;
            }
            break;

        default: break;
    }

    type->kind = TYPE_UNKNOWN;
    type->size = 0;
    type->align = 1;
}

void free_types(void)
{
    free_type(type_unknown);
    free(type_unknown);
    type_unknown = NULL;

    free_type(type_error);
    free(type_error);
    type_error = NULL;

    free_type(type_void);
    free(type_void);
    type_void = NULL;

    free_type(type_bool);
    free(type_bool);
    type_bool = NULL;

    free_type(type_int);
    free(type_int);
    type_int = NULL;

    free_type(type_uint);
    free(type_uint);
    type_uint = NULL;

    free_type(type_float);
    free(type_float);
    type_float = NULL;

    free_type(type_str);
    free(type_str);
    type_str = NULL;

    free_type(type_char);
    free(type_char);
    type_char = NULL;
}
