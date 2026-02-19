#include <stdlib.h>
#include <stdalign.h>

#include "compiler/frontend/semantic/types.h"

#define DEF_TYPE_SIZE  0
#define DEF_TYPE_ALIGN 1

type_t* type_unknown = NULL;
type_t* type_error = NULL;
type_t* type_void = NULL;
type_t* type_any = NULL;
type_t* type_bool = NULL;
type_t* type_int = NULL;
type_t* type_uint = NULL;
type_t* type_short = NULL;
type_t* type_ushort = NULL;
type_t* type_long = NULL;
type_t* type_ulong = NULL;
type_t* type_float = NULL;
type_t* type_decimal = NULL;
type_t* type_str = NULL;
type_t* type_char = NULL;

type_t* new_type(arena_t* arena, const enum type_kind kind, const size_t size, const size_t align)
{
    type_t* type = arena_alloc(arena, sizeof(type_t), alignof(type_t));
    if(!type) return NULL;
    type->kind = kind;
    type->size = size;
    type->align = align;
    return type;
}

void init_types(arena_t* arena)
{
    type_unknown = new_type(arena, TYPE_UNKNOWN, DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    type_error =   new_type(arena, TYPE_ERROR,   DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    type_void =    new_type(arena, TYPE_VOID,    DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    type_any =     new_type(arena, TYPE_ANY,   sizeof(void*), alignof(void*));
    type_bool =    new_type(arena, TYPE_BOOL,  sizeof(bool),  alignof(bool));
    type_int =     new_type(arena, TYPE_INT,   sizeof(int),   alignof(int));
    type_uint =    new_type(arena, TYPE_UINT,  sizeof(unsigned int), alignof(unsigned int));
    type_short =   new_type(arena, TYPE_INT,   sizeof(short), alignof(short));
    type_ushort =  new_type(arena, TYPE_UINT,  sizeof(unsigned short), alignof(unsigned short));
    type_long =    new_type(arena, TYPE_INT,   sizeof(long),  alignof(long));
    type_ulong =   new_type(arena, TYPE_UINT,  sizeof(unsigned long), alignof(unsigned long));
    type_float =   new_type(arena, TYPE_FLOAT, sizeof(float), alignof(float));
    type_decimal = new_type(arena, TYPE_FLOAT, sizeof(long),  alignof(long));
    type_str =     new_type(arena, TYPE_STR,   sizeof(char*), alignof(char*));
    type_char =    new_type(arena, TYPE_CHAR,  sizeof(char),  alignof(char));
}

type_t* new_type_array(arena_t* arena, type_t* elem_type, const size_t length)
{
    type_t* type = new_type(arena, TYPE_ARRAY, DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    if(!type) return NULL;

    type->array.elem_type = elem_type;
    type->array.length = length;

    if(length > 0 && elem_type){
        type->size = elem_type->size * length;
        type->align = elem_type->align;
    }
    else {
        type->size = sizeof(void*);
        type->align = sizeof(void*);
    }

    return type;
}

type_t* new_type_function(arena_t* arena, type_t* return_type, type_t** param_types, const size_t param_count)
{
    type_t* type = new_type(arena, TYPE_FUNC, DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    if(!type) return NULL;

    type->func.return_type = return_type;
    type->func.param_types = param_types;
    type->func.param_count = param_count;

    if(return_type){
        type->size = return_type->size;
        type->align = return_type->align;
    }
    else {
        type->size = sizeof(void*);
        type->align = sizeof(void*);
    }

    return type;
}

type_t* new_type_compound(arena_t* arena, enum type_kind kind, struct symbol* scope, const size_t member_count)
{
    type_t* type = new_type(arena, kind, DEF_TYPE_SIZE, DEF_TYPE_ALIGN);
    if(!type) return NULL;
    type->compound.scope = scope;
    type->compound.member_count = member_count;
    return type;
}

bool types_equal(const type_t* a, const type_t* b)
{
    if(!a || !b) return false;
    if(a->kind != b->kind) return false;

    switch(a->kind){
        case TYPE_ARRAY:
            return types_equal(a->array.elem_type, b->array.elem_type) && (a->array.length == b->array.length);

        case TYPE_FUNC:
            if(!types_equal(a->func.return_type, b->func.return_type)) return false;
            if(a->func.param_count != b->func.param_count) return false;
            for(size_t i = 0; i < a->func.param_count; ++i){
                if(!types_equal(a->func.param_types[i], b->func.param_types[i])){
                    return false;
                }
            }
            return true;

        case TYPE_STRUCT:
            return a->compound.member_count == b->compound.member_count;

        default: return true;
    }
}

inline bool is_type(type_t* expected, type_t* actual, enum type_kind kind)
{
    return expected->kind == kind && actual->kind == kind;
}

bool types_compatible(const type_t* a, const type_t* b)
{
    if(!a || !b) return false;

    if(types_equal(a, b)) return true;

    // "any" and all types are compatible
    if(a->kind == TYPE_ANY){
        switch(b->kind){
            case TYPE_VOID:
            case TYPE_BOOL:
            case TYPE_INT:
            case TYPE_UINT:
            case TYPE_SHORT:
            case TYPE_USHORT:
            case TYPE_LONG:
            case TYPE_ULONG:
            case TYPE_FLOAT:
            case TYPE_DECIMAL:
            case TYPE_STR:
            case TYPE_CHAR:
            case TYPE_ARRAY:
            case TYPE_FUNC:
            case TYPE_STRUCT:
            case TYPE_ENUM:
                return true;
            default:
                return false;
        }
    }

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
    return(type->kind == TYPE_INT
        || type->kind == TYPE_UINT
        || type->kind == TYPE_SHORT
        || type->kind == TYPE_USHORT
        || type->kind == TYPE_LONG
        || type->kind == TYPE_ULONG
        || type->kind == TYPE_FLOAT);
}

bool is_integer_type(const type_t* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT || type->kind == TYPE_UINT;
}

bool is_signed_type(const type_t* type)
{
    if(!type) return false;
    return type->kind == TYPE_INT
        || type->kind == TYPE_SHORT
        || type->kind == TYPE_LONG
        || type->kind == TYPE_FLOAT;
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

        case TYPE_FUNC:
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
            if(type->compound.scope){
                // free_scope(type->compound.scope);
            }
            break;

        default: break;
    }

    type->kind = TYPE_UNKNOWN;
    type->size = DEF_TYPE_SIZE;
    type->align = DEF_TYPE_ALIGN;
}
