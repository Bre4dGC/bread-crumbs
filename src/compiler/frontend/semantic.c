#include <stdlib.h>

#include "compiler/core/arena_alloc.h"
#include "compiler/core/diagnostic.h"
#include "compiler/frontend/tokenizer.h"
#include "compiler/frontend/types.h"
#include "compiler/frontend/semantic.h"

bool check_node(semantic_t* sem, node_t* node);
bool check_function(semantic_t* sem, node_t* node);
bool check_variable(semantic_t* sem, node_t* node);
bool check_param(semantic_t* sem, node_t* node);
bool check_block(semantic_t* sem, node_t* node);
bool check_if(semantic_t* sem, node_t* node);
bool check_while(semantic_t* sem, node_t* node);
bool check_for(semantic_t* sem, node_t* node);
bool check_return(semantic_t* sem, node_t* node);
bool check_break(semantic_t* sem, node_t* node);
bool check_continue(semantic_t* sem, node_t* node);
bool check_expression(semantic_t* sem, node_t* node);
bool check_binary_op(semantic_t* sem, node_t* node);
bool check_unary_op(semantic_t* sem, node_t* node);
bool check_func_call(semantic_t* sem, node_t* node);
bool check_var_ref(semantic_t* sem, node_t* node);
bool check_literal(semantic_t* sem, node_t* node);
bool check_array(semantic_t* sem, node_t* node);
bool check_struct(semantic_t* sem, node_t* node);
bool check_enum(semantic_t* sem, node_t* node);

type_t* infer_type(semantic_t* sem, node_t* node);

bool check_type_compatibility(semantic_t* sem, node_t* node, type_t* expected, type_t* actual);

bool all_paths_return(node_t* node);
bool is_unreachable_code(node_t* node);

semantic_t* new_semantic(arena_t* arena, string_pool_t* string_pool, report_table_t* reports)
{
    semantic_t* sem = (semantic_t*)arena_alloc(arena, sizeof(semantic_t), alignof(semantic_t));
    if(!sem) return NULL;

    // initialize built-in types if not already done
    if(type_int == NULL) init_types(arena);

    sem->symbols = new_symbol_table(arena, string_pool);
    if(!sem->symbols) return NULL;

    sem->current_function = NULL;
    sem->loop_depth = 0;
    sem->phase = PHASE_DECLARE;

    sem->arena = arena;
    sem->reports = reports;

    return sem;
}

bool analyze_ast(semantic_t* sem, node_t* root)
{
    if(!sem || !root) return false;

    // declare top-level symbols (functions/types).
    sem->phase = PHASE_DECLARE;
    if(root->kind == NODE_BLOCK){
        for(size_t i = 0; i < root->block->statement.count; i++){
            node_t* stmt = root->block->statement.elems[i];
            if(!stmt) continue;
            if(stmt->kind == NODE_FUNC || stmt->kind == NODE_STRUCT || stmt->kind == NODE_ENUM){
                (void)check_node(sem, stmt);
            }
        }
    }
    else {
        if(root->kind == NODE_FUNC || root->kind == NODE_STRUCT || root->kind == NODE_ENUM){
            (void)check_node(sem, root);
        }
    }

    // full semantic checks.
    sem->phase = PHASE_CHECK;
    if(root->kind == NODE_BLOCK){
        bool ok = true;
        for(size_t i = 0; i < root->block->statement.count; i++){
            ok = check_node(sem, root->block->statement.elems[i]) && ok;
        }
        return ok;
    }

    return check_node(sem, root);
}

void free_semantic(semantic_t* sem)
{
    if(!sem) return;
    if(sem->symbols) free_symbol_table(sem->symbols);
    sem->symbols = NULL;
}

bool check_node(semantic_t* sem, node_t* node)
{
    if(!sem || !node) return false;

    switch(node->kind){
        case NODE_VAR:      return check_variable(sem, node);
        case NODE_REF:      return check_var_ref(sem, node);
        case NODE_LITERAL:  return check_literal(sem, node);
        case NODE_BINOP:    return check_binary_op(sem, node);
        case NODE_UNARYOP:  return check_unary_op(sem, node);
        case NODE_CALL:     return check_func_call(sem, node);
        case NODE_BLOCK:    return check_block(sem, node);
        case NODE_IF:       return check_if(sem, node);
        case NODE_WHILE:    return check_while(sem, node);
        case NODE_FOR:      return check_for(sem, node);
        case NODE_RETURN:   return check_return(sem, node);
        case NODE_BREAK:    return check_break(sem, node);
        case NODE_CONTINUE: return check_continue(sem, node);
        case NODE_FUNC:     return check_function(sem, node);
        case NODE_ARRAY:    return check_array(sem, node);
        case NODE_STRUCT:   return check_struct(sem, node);
        case NODE_ENUM:     return check_enum(sem, node);
        default:
            add_report(sem->reports, SEV_WARN, ERR_UNIMPL_NODE, node->loc, DEFAULT_LEN, NULL);
            return true;
    }
}

type_t* datatype_to_type(int dtype)
{
    switch(dtype){
        case DT_VOID:   return type_void;
        case DT_ANY:    return type_any;
        case DT_BOOL:   return type_bool;
        case DT_INT:    return type_int;
        case DT_UINT:   return type_uint;
        case DT_FLOAT:  return type_float;
        case DT_STR:    return type_str;
        default:        return type_unknown;
    }
}

bool check_function(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_FUNC) return false;

    struct node_func* func = node->func_decl;
    if(!func || !func->name.data) return false;

    // register the function symbol (no body checking yet)
    if(sem->phase == PHASE_DECLARE){
        if(is_scope_symbol_exist(sem->symbols, func->name.data)){
            add_report(sem->reports, SEV_ERR, ERR_FUNC_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
            return false;
        }

        // build a function type from signature
        type_t* return_type = datatype_to_type(func->return_type);
        size_t param_count = func->param.count;
        type_t** param_types = NULL;
        if(param_count > 0){
            param_types = (type_t**)arena_alloc_array(sem->arena, sizeof(type_t*), param_count, alignof(type_t*));
            if(!param_types) return false;
            for(size_t i = 0; i < param_count; i++){
                node_t* p = func->param.elems[i];
                if(p && p->kind == NODE_VAR && p->var_decl){
                    int dt = p->var_decl->dtype;
                    param_types[i] = (dt == DT_VOID) ? type_any : datatype_to_type(dt);
                }
                else {
                    param_types[i] = type_any;
                }
            }
        }
        type_t* func_type = new_type_function(sem->arena, return_type, param_types, param_count);

        symbol_t* func_sym = define_symbol(sem->symbols, func->name.data, SYMBOL_FUNC, func_type, node);
        if(!func_sym){
            add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_FUNC, node->loc, DEFAULT_LEN, NULL);
            return false;
        }

        return true;
    }

    // check function body
    symbol_t* func_sym = lookup_symbol(sem->symbols, func->name.data);
    if(!func_sym){
        type_t* return_type = datatype_to_type(func->return_type);
        type_t* func_type = new_type_function(sem->arena, return_type, NULL, 0);
        func_sym = define_symbol(sem->symbols, func->name.data, SYMBOL_FUNC, func_type, node);
        if(!func_sym) return false;
    }

    // create new scope for function body
    push_scope(sem->symbols, SCOPE_FUNCTION, node);
    symbol_t* prev_func = sem->current_function;
    sem->current_function = func_sym;

    // add parameters to function scope
    for(size_t i = 0; i < func->param.count; i++){
        if(!check_param(sem, func->param.elems[i])){
            pop_scope(sem->symbols);
            sem->current_function = prev_func;
            return false;
        }
    }

    // check function body
    bool success = true;
    if(func->body){
        success = check_node(sem, func->body);
    }

    // TODO: Check that all paths return if return type is not void

    pop_scope(sem->symbols);
    sem->current_function = prev_func;
    return success;
}

bool check_param(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_VAR || !node->var_decl) return false;

    struct node_var* var = node->var_decl;
    if(!var->name.data) return false;

    if(is_scope_symbol_exist(sem->symbols, var->name.data)){
        add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    // determine parameter type
    type_t* param_type = (var->dtype == DT_VOID) ? type_any : datatype_to_type(var->dtype);
    symbol_t* sym = define_symbol(sem->symbols, var->name.data, SYMBOL_PARAM, param_type, node);
    if(!sym){
        add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, node->loc, DEFAULT_LEN, NULL);
        return false;
    }
    sym->flags |= SYM_FLAG_ASSIGNED;
    return true;
}

bool check_variable(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_VAR) return false;

    struct node_var* var = node->var_decl;
    if(!var || !var->name.data) return false;

    if(is_scope_symbol_exist(sem->symbols, var->name.data)){
        add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    type_t* var_type = NULL; // determine

    if(var->dtype != DT_VOID){
        var_type = datatype_to_type(var->dtype); // explicit type annotation
    }
    else if(var->value){
        // type inference from initializer
        if(!check_node(sem, var->value)) return false;
        var_type = infer_type(sem, var->value);
    }
    else {
        // no type and no initializer
        add_report(sem->reports, SEV_ERR, ERR_VAR_NO_TYPE_OR_INITIALIZER, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    if(!var_type || var_type == type_error){
        add_report(sem->reports, SEV_ERR, ERR_VAR_NO_TYPE_OR_INITIALIZER, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    // check type compatibility if both annotation and initializer exist
    if(var->dtype != DT_VOID && var->value){
        type_t* init_type = infer_type(sem, var->value);
        if(!check_type_compatibility(sem, node, var_type, init_type)){
            add_report(sem->reports, SEV_ERR, ERR_TYPE_MISMATCH, node->loc, DEFAULT_LEN, NULL);
            return false;
        }
    }

    // determine symbol kind
    enum symbol_kind kind = SYMBOL_VAR;
    if(var->modif == MOD_CONST || var->modif == MOD_FINAL){
        kind = SYMBOL_CONST;
    }

    // add to symbol table
    symbol_t* sym = define_symbol(sem->symbols, var->name.data, kind, var_type, node);
    if(!sym){
        add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    // mark as initialized if has value
    if(var->value) sym->flags |= SYM_FLAG_ASSIGNED;

    return true;
}

bool check_block(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_BLOCK) return false;

    push_scope(sem->symbols, SCOPE_BLOCK, node); // create new scope for block

    bool success = true;
    for(size_t i = 0; i < node->block->statement.count; i++){
        if(!check_node(sem, node->block->statement.elems[i])){
            success = false;
            // TODO: continue checking other statements
        }
    }

    pop_scope(sem->symbols);
    return success;
}

bool check_if(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_IF) return false;

    if(!check_node(sem, node->if_stmt->condition)) return false; // check condition

    // check branches
    bool success = true;
    if(node->if_stmt->then_block)  success = check_node(sem, node->if_stmt->then_block)  && success;
    if(node->if_stmt->else_block)  success = check_node(sem, node->if_stmt->else_block)  && success;
    if(node->if_stmt->elif_blocks) success = check_node(sem, node->if_stmt->elif_blocks) && success;

    return success;
}

bool check_while(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_WHILE) return false;

    sem->loop_depth++;

    bool success = true;
    if(node->while_loop->condition) success = check_node(sem, node->while_loop->condition) && success;
    if(node->while_loop->body)      success = check_node(sem, node->while_loop->body) && success;

    sem->loop_depth--;
    return success;
}

bool check_for(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_FOR) return false;

    push_scope(sem->symbols, SCOPE_BLOCK, node);
    sem->loop_depth++;

    bool success = true;
    if(node->for_loop->init)      success = check_node(sem, node->for_loop->init) && success;
    if(node->for_loop->condition) success = check_node(sem, node->for_loop->condition) && success;
    if(node->for_loop->update)    success = check_node(sem, node->for_loop->update) && success;
    if(node->for_loop->body)      success = check_node(sem, node->for_loop->body) && success;

    sem->loop_depth--;
    pop_scope(sem->symbols);
    return success;
}

bool check_return(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_RETURN) return false;

    if(!sem->current_function){
        add_report(sem->reports, SEV_ERR, ERR_RET_OUTSIDE_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    if(node->ret->body){
        return check_node(sem, node->ret->body);
    }

    return true;
}

bool check_break(semantic_t* sem, node_t* node)
{
    if(!sem || !node) return false;

    if(sem->loop_depth == 0){
        add_report(sem->reports, SEV_ERR, ERR_BREAK_OUTSIDE_LOOP, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    return true;
}

bool check_continue(semantic_t* sem, node_t* node)
{
    if(!sem || !node) return false;

    if(sem->loop_depth == 0){
        add_report(sem->reports, SEV_ERR, ERR_CONTINUE_OUTSIDE_LOOP, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    return true;
}

bool check_expression(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_EXPR) return false;
    // TODO: Implement expression checking
    return true;
}

bool check_binary_op(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_BINOP) return false;

    // check both operands
    if(!check_node(sem, node->binop->left)) return false;
    if(!check_node(sem, node->binop->right)) return false;

    // infer types
    type_t* left_type = infer_type(sem, node->binop->left);
    type_t* right_type = infer_type(sem, node->binop->right);

    if(!left_type || left_type == type_error || !right_type || right_type == type_error){
        return false;
    }

    // check type compatibility for operation
    // enum op_code op = node->bin_op.code;
    // (void)op;  // TODO: use for operator-specific checks

    if(!types_compatible(left_type, right_type)){
        add_report(sem->reports, SEV_ERR, ERR_TYPE_MISMATCH, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    return true;
}

bool check_unary_op(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_UNARYOP) return false;

    return check_node(sem, node->unaryop->right);
}

bool check_func_call(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_CALL) return false;

    // Lookup function
    symbol_t* func_sym = lookup_symbol(sem->symbols, node->call->name.data);
    if(!func_sym){
        add_report(sem->reports, SEV_ERR, ERR_UNDEC_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    if(func_sym->kind != SYMBOL_FUNC){
        add_report(sem->reports, SEV_ERR, ERR_NOT_A_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    for(size_t i = 0; i < node->call->args.count; i++){
        if(!check_node(sem, node->call->args.elems[i])) return false;
    }

    // TODO: Check argument count and types match parameters

    func_sym->flags |= SYM_FLAG_USED;
    return true;
}

bool check_var_ref(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_REF) return false;

    const char* name = node->var_ref->name.data;
    if(!name) return false;

    // lookup variable
    symbol_t* sym = lookup_symbol(sem->symbols, name);
    if(!sym){
        add_report(sem->reports, SEV_ERR, ERR_UNDEC_VAR, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    sym->flags |= SYM_FLAG_USED;

    return true;
}

bool check_literal(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_LITERAL) return false;
    // literals are always valid
    return true;
}

bool check_array(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_ARRAY) return false;

    for(size_t i = 0; i < node->array_decl->count; i++){
        if(!check_node(sem, node->array_decl->elements[i])){
            return false;
        }
    }

    return true;
}

bool check_struct(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_STRUCT) return false;
    // TODO: Implement struct checking
    return true;
}

bool check_enum(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_ENUM) return false;
    // TODO: Implement enum checking
    return true;
}


type_t* infer_type(semantic_t* sem, node_t* node)
{
    if(!sem || !node) return type_error;

    switch(node->kind){
        case NODE_LITERAL:
            switch(node->lit->type){
                case LIT_NUMBER:
                case LIT_BIN:
                case LIT_HEX:
                    return type_int;
                case LIT_FLOAT:
                    return type_float;
                case LIT_STRING:
                    return type_str;
                case LIT_TRUE:
                case LIT_FALSE:
                    return type_bool;
                case LIT_NULL:
                    return type_void;
                default:
                    return type_unknown;
            }

        case NODE_REF: {
            symbol_t* sym = lookup_symbol(sem->symbols, node->var_ref->name.data);
            return sym ? sym->type : type_error;
        }

        case NODE_CALL: {
            symbol_t* func = lookup_symbol(sem->symbols, node->call->name.data);
            if(func && func->type && func->type->kind == TYPE_FUNCTION){
                return func->type->func.return_type;
            }
            return type_error;
        }

        default: return type_unknown;
    }
}

inline bool is_type(type_t* expected, type_t* actual, enum type_kind kind)
{
    return expected->kind == kind && actual->kind == kind;
}

bool check_type_compatibility(semantic_t* sem, node_t* node, type_t* expected, type_t* actual)
{
    if(!sem || !node || !expected || !actual) return false;

    if(is_type(expected, actual, TYPE_ANY)) return true;
    if(is_type(expected, actual, TYPE_ERROR)) return false;
    if(is_type(expected, actual, TYPE_VOID)) return true;
    if(is_type(expected, actual, TYPE_BOOL)) return true;
    if(is_type(expected, actual, TYPE_INT)) return true;
    if(is_type(expected, actual, TYPE_UINT)) return true;
    if(is_type(expected, actual, TYPE_FLOAT)) return true;
    if(is_type(expected, actual, TYPE_STR)) return true;
    if(is_type(expected, actual, TYPE_CHAR)) return true;
    if(is_type(expected, actual, TYPE_ARRAY)) return true;
    if(is_type(expected, actual, TYPE_FUNCTION)) return true;
    if(is_type(expected, actual, TYPE_STRUCT)) return true;
    if(is_type(expected, actual, TYPE_ENUM)) return true;
    if(is_type(expected, actual, TYPE_UNION)) return true;

    return types_compatible(expected, actual);
}

bool all_paths_return(node_t* node)
{
    // TODO: Implement control flow analysis
    (void)node;
    return true;
}

bool is_unreachable_code(node_t* node)
{
    // TODO: Implement unreachable code detection
    (void)node;
    return false;
}
