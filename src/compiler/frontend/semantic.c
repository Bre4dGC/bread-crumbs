#include <stdlib.h>

#include "core/arena.h"
#include "core/diagnostic.h"
#include "compiler/frontend/lexer/tokens.h"
#include "compiler/frontend/semantic.h"
#ifdef DEBUG
#include "core/common/debug.h"
#endif

type_t* infer_type(semantic_t* sem, node_t* node);

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
bool check_expr(semantic_t* sem, node_t* node);
bool check_binop(semantic_t* sem, node_t* node);
bool check_unaryop(semantic_t* sem, node_t* node);
bool check_func_call(semantic_t* sem, node_t* node);
bool check_var_ref(semantic_t* sem, node_t* node);
bool check_literal(semantic_t* sem, node_t* node);
bool check_array(semantic_t* sem, node_t* node);
bool check_struct(semantic_t* sem, node_t* node);
bool check_enum(semantic_t* sem, node_t* node);

bool check_type_compatibility(semantic_t* sem, node_t* node, type_t* expected, type_t* actual);

bool all_paths_return(node_t* node);
bool is_unreachable_code(node_t* node);

semantic_t* new_semantic(arena_t* arena, string_pool_t* string_pool, report_table_t* reports)
{
    if(!arena || !string_pool || !reports) {
        return NULL;
    }

    semantic_t* sem = arena_alloc(arena, sizeof(semantic_t), alignof(semantic_t));
    if(!sem) {
        return NULL;
    }

    // init if not already done
    if(!type_int) {
        init_types(arena);
    }

    sem->symbols = new_symbol_table(arena, string_pool);
    if(!sem->symbols) {
        return NULL;
    }

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

    // declare top-level symbols
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

#ifdef DEBUG
        print_symbol_table(sem->symbols);
#endif

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
        case NODE_BINOP:    return check_binop(sem, node);
        case NODE_UNARYOP:  return check_unaryop(sem, node);
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

    // register the function symbol
    if(sem->phase == PHASE_DECLARE){
        if(is_scope_symbol_exist(sem->symbols, func->name.data)){
            add_report(sem->reports, SEV_ERR, ERR_FUNC_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
            return false;
        }

        // build a function type from signature
        type_t* return_type = datatype_to_type(func->return_type);
        size_t param_count = func->param_decl.count;
        type_t** param_types = NULL;
        if(param_count > 0){
            param_types = arena_alloc_array(sem->arena, sizeof(type_t*), param_count, alignof(type_t*));
            if(!param_types) return false;
            for(size_t i = 0; i < param_count; i++){
                node_t* p = func->param_decl.elems[i];
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

#ifdef DEBUG
        print_symbol(func_sym, 0);
#endif
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

    // create new function body scope
    scope_t* function_scope = push_scope(sem->symbols, SCOPE_FUNCTION, node);
    if(!function_scope) return false;

#ifdef DEBUG
    print_current_scope(sem->symbols);
#endif

    symbol_t* prev_func = sem->current_function;
    sem->current_function = func_sym;

    // add parameters to function scope
    bool params_ok = true;
    for(size_t i = 0; i < func->param_decl.count; i++){
        if(!check_param(sem, func->param_decl.elems[i])){
            params_ok = false;
            break;
        }
    }

    // check function body
    bool success = true;
    if(params_ok && func->body){
        success = check_node(sem, func->body);
    }

    // TODO: check that all paths return if return type is not void

    pop_scope(sem->symbols);
#ifdef DEBUG
    print_symbol_table(sem->symbols);
#endif
    sem->current_function = prev_func;
    return success && params_ok;
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

#ifdef DEBUG
    print_symbol(sym, 0);
#endif
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

#ifdef DEBUG
    print_symbol(sym, 0);
#endif

    // mark as initialized if has value
    if(var->value) sym->flags |= SYM_FLAG_ASSIGNED;

    return true;
}

bool check_block(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_BLOCK) return false;

    scope_t* block_scope = push_scope(sem->symbols, SCOPE_BLOCK, node);
    if(!block_scope) return false;

    bool success = true;
    for(size_t i = 0; i < node->block->statement.count; i++){
        if(!check_node(sem, node->block->statement.elems[i])){
            success = false;
            // TODO: continue checking other statements
        }
    }

    pop_scope(sem->symbols);
#ifdef DEBUG
    print_current_scope(sem->symbols);
#endif
    return success;
}

bool check_if(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_IF) return false;

    if(!check_node(sem, node->if_stmt->condition)) return false; // check condition

#ifdef DEBUG
    print_current_scope(sem->symbols);
    // printf("%d", sem.)
#endif

    // check branches
    bool success = true;
    if(node->if_stmt->then_block)  success = check_node(sem, node->if_stmt->then_block)  && success;
    if(node->if_stmt->elif_blocks) success = check_node(sem, node->if_stmt->elif_blocks) && success;
    if(node->if_stmt->else_block)  success = check_node(sem, node->if_stmt->else_block)  && success;

    return success;
}

bool check_while(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_WHILE) return false;

    sem->loop_depth++;

    bool success = true;
    if(node->while_stmt->condition) success = check_node(sem, node->while_stmt->condition) && success;
    if(node->while_stmt->body)      success = check_node(sem, node->while_stmt->body) && success;

    sem->loop_depth--;
#ifdef DEBUG
    print_current_scope(sem->symbols);
#endif
    return success;
}

bool check_for(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_FOR) return false;

    scope_t* for_scope = push_scope(sem->symbols, SCOPE_BLOCK, node);
    if(!for_scope) return false;

    sem->loop_depth++;

    bool success = true;
    if(node->for_stmt->init)      success = check_node(sem, node->for_stmt->init) && success;
    if(node->for_stmt->condition) success = check_node(sem, node->for_stmt->condition) && success;
    if(node->for_stmt->update)    success = check_node(sem, node->for_stmt->update) && success;
    if(node->for_stmt->body)      success = check_node(sem, node->for_stmt->body) && success;

    sem->loop_depth--;
    pop_scope(sem->symbols);
#ifdef DEBUG
    print_current_scope(sem->symbols);
#endif
    return success;
}

bool check_return(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_RETURN) return false;

    if(!sem->current_function){
        add_report(sem->reports, SEV_ERR, ERR_RET_OUTSIDE_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    if(node->return_stmt->body){
        return check_node(sem, node->return_stmt->body);
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

bool check_expr(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_EXPR) return false;
    // TODO: implement expression checking
    return true;
}

bool check_binop(semantic_t* sem, node_t* node)
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

bool check_unaryop(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_UNARYOP) return false;

    return check_node(sem, node->unaryop->right);
}

bool check_func_call(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_CALL) return false;

    // lookup function
    symbol_t* func_sym = lookup_symbol(sem->symbols, node->func_call->name.data);
    if(!func_sym){
        add_report(sem->reports, SEV_ERR, ERR_UNDEC_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

#ifdef DEBUG
    print_symbol(func_sym, 0);
#endif

    if(func_sym->kind != SYMBOL_FUNC){
        add_report(sem->reports, SEV_ERR, ERR_NOT_A_FUNC, node->loc, DEFAULT_LEN, NULL);
        return false;
    }

    for(size_t i = 0; i < node->func_call->args.count; i++){
        if(!check_node(sem, node->func_call->args.elems[i])) return false;
    }

    // TODO: check argument count and types match parameters

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

#ifdef DEBUG
    print_symbol(sym, 0);
#endif

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
    
    struct node_struct* struct_decl = node->struct_decl;
    if(!struct_decl || !struct_decl->name.data) return false;

    // register struct symbol in declare phase
    if(sem->phase == PHASE_DECLARE){
        if(is_scope_symbol_exist(sem->symbols, struct_decl->name.data)){
            add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
            return false;
        }

        // create struct type (will be populated in check phase)
        type_t* struct_type = new_type_compound(sem->arena, TYPE_STRUCT, NULL, 0);
        symbol_t* struct_sym = define_symbol(sem->symbols, struct_decl->name.data, SYMBOL_STRUCT, struct_type, node);
        if(!struct_sym){
            add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, node->loc, DEFAULT_LEN, NULL);
            return false;
        }
        return true;
    }

    // check struct members
    symbol_t* struct_sym = lookup_symbol(sem->symbols, struct_decl->name.data);
    if(!struct_sym) return false;

    scope_t* struct_scope = push_scope(sem->symbols, SCOPE_STRUCT, node);
    if(!struct_scope) return false;
    
    bool success = true;
    size_t member_count = 0;
    
    for(size_t i = 0; i < struct_decl->member.count; i++){
        node_t* member = struct_decl->member.elems[i];
        if(!member || member->kind != NODE_VAR) {
            add_report(sem->reports, SEV_ERR, ERR_INVAL_EXPR, member ? member->loc : node->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        struct node_var* var = member->var_decl;
        if(!var || !var->name.data) {
            success = false;
            continue;
        }

        // check for duplicate member names
        if(is_scope_symbol_exist(sem->symbols, var->name.data)){
            add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        // determine member type
        type_t* member_type = NULL;
        if(var->dtype != DT_VOID){
            member_type = datatype_to_type(var->dtype);
        } else if(var->value) {
            if(!check_node(sem, var->value)) {
                success = false;
                continue;
            }
            member_type = infer_type(sem, var->value);
        } else {
            add_report(sem->reports, SEV_ERR, ERR_VAR_NO_TYPE_OR_INITIALIZER, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        if(!member_type || member_type == type_error) {
            add_report(sem->reports, SEV_ERR, ERR_VAR_NO_TYPE_OR_INITIALIZER, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        // create member symbol
        symbol_t* member_sym = define_symbol(sem->symbols, var->name.data, SYMBOL_VAR, member_type, member);
        if(!member_sym) {
            add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        member_sym->flags |= SYM_FLAG_ASSIGNED; // struct members are always "assigned"
        member_count++;
    }

    // update struct type with member information
    if(struct_sym->type && struct_sym->type->kind == TYPE_STRUCT){
        struct_sym->type->compound.scope = struct_scope->symbols ? (struct symbol*)struct_scope : NULL;
        struct_sym->type->compound.member_count = member_count;
    }
    
    pop_scope(sem->symbols);
    return success;
}

bool check_enum(semantic_t* sem, node_t* node)
{
    if(!sem || !node || node->kind != NODE_ENUM) return false;
    
    struct node_enum* enum_decl = node->enum_decl;
    if(!enum_decl || !enum_decl->name.data) return false;
    
    // register enum symbol in declare phase
    if(sem->phase == PHASE_DECLARE){
        if(is_scope_symbol_exist(sem->symbols, enum_decl->name.data)){
            add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, node->loc, DEFAULT_LEN, NULL);
            return false;
        }

        // create enum type
        type_t* enum_type = new_type_compound(sem->arena, TYPE_ENUM, NULL, 0);
        symbol_t* enum_sym = define_symbol(sem->symbols, enum_decl->name.data, SYMBOL_ENUM, enum_type, node);
        if(!enum_sym){
            add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, node->loc, DEFAULT_LEN, NULL);
            return false;
        }
        return true;
    }

    // check enum variants
    symbol_t* enum_sym = lookup_symbol(sem->symbols, enum_decl->name.data);
    if(!enum_sym) return false;

    scope_t* enum_scope = push_scope(sem->symbols, SCOPE_ENUM, node);
    if(!enum_scope) return false;
    
    bool success = true;
    size_t variant_count = 0;
    int next_value = 0; // auto-increment values
    
    for(size_t i = 0; i < enum_decl->member.count; i++){
        node_t* member = enum_decl->member.elems[i];
        if(!member) {
            success = false;
            continue;
        }

        const char* variant_name = NULL;
        int variant_value = next_value;

        // handle different enum member formats
        if(member->kind == NODE_VAR && member->var_decl) {
            variant_name = member->var_decl->name.data;
            if(member->var_decl->value) {
                // explicit value assignment
                if(member->var_decl->value->kind == NODE_LITERAL) {
                    variant_value = atoi(member->var_decl->value->lit->value.data);
                } else {
                    add_report(sem->reports, SEV_ERR, ERR_INVAL_EXPR, member->var_decl->value->loc, DEFAULT_LEN, NULL);
                    success = false;
                    continue;
                }
            }
        } else if(member->kind == NODE_REF && member->var_ref) {
            variant_name = member->var_ref->name.data;
        } else {
            add_report(sem->reports, SEV_ERR, ERR_INVAL_EXPR, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        if(!variant_name) {
            success = false;
            continue;
        }

        // check for duplicate variant names
        if(is_scope_symbol_exist(sem->symbols, variant_name)){
            add_report(sem->reports, SEV_ERR, ERR_VAR_ALREADY_DECL, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        // create variant symbol with int type
        symbol_t* variant_sym = define_symbol(sem->symbols, variant_name, SYMBOL_ENUM_VARIANT, type_int, member);
        if(!variant_sym) {
            add_report(sem->reports, SEV_ERR, ERR_FAIL_TO_DECL_VAR, member->loc, DEFAULT_LEN, NULL);
            success = false;
            continue;
        }

        variant_sym->flags |= SYM_FLAG_ASSIGNED;
        variant_count++;
        next_value = variant_value + 1;
    }

    // update enum type with variant information
    if(enum_sym->type && enum_sym->type->kind == TYPE_ENUM){
        enum_sym->type->compound.scope = enum_scope->symbols ? (struct symbol*)enum_scope : NULL;
        enum_sym->type->compound.member_count = variant_count;
    }
    
    pop_scope(sem->symbols);
    return success;
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
            symbol_t* func = lookup_symbol(sem->symbols, node->func_call->name.data);
            if(func && func->type && func->type->kind == TYPE_FUNC){
                return func->type->func.return_type;
            }
            return type_error;
        }

        default: return type_unknown;
    }
}

bool check_type_compatibility(semantic_t* sem, node_t* node, type_t* expected, type_t* actual)
{
    if(!sem || !node || !expected || !actual) return false;
    return types_compatible(expected, actual);
}

bool all_paths_return(node_t* node)
{
    // TODO: implement control flow analysis
    (void)node;
    return true;
}

bool is_unreachable_code(node_t* node)
{
    // TODO: implement unreachable code detection
    (void)node;
    return false;
}
