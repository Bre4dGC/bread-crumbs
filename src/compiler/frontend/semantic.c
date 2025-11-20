#include "compiler/frontend/semantic.h"

bool check_node(semantic_context_t* ctx, astnode_t* node);
bool check_function(semantic_context_t* ctx, astnode_t* node);
bool check_variable(semantic_context_t* ctx, astnode_t* node);
bool check_block(semantic_context_t* ctx, astnode_t* node);
bool check_if(semantic_context_t* ctx, astnode_t* node);
bool check_while(semantic_context_t* ctx, astnode_t* node);
bool check_for(semantic_context_t* ctx, astnode_t* node);
bool check_return(semantic_context_t* ctx, astnode_t* node);
bool check_break(semantic_context_t* ctx, astnode_t* node);
bool check_continue(semantic_context_t* ctx, astnode_t* node);
bool check_expression(semantic_context_t* ctx, astnode_t* node);
bool check_binary_op(semantic_context_t* ctx, astnode_t* node);
bool check_unary_op(semantic_context_t* ctx, astnode_t* node);
bool check_func_call(semantic_context_t* ctx, astnode_t* node);
bool check_var_ref(semantic_context_t* ctx, astnode_t* node);
bool check_literal(semantic_context_t* ctx, astnode_t* node);
bool check_array(semantic_context_t* ctx, astnode_t* node);
bool check_struct(semantic_context_t* ctx, astnode_t* node);
bool check_union(semantic_context_t* ctx, astnode_t* node);
bool check_enum(semantic_context_t* ctx, astnode_t* node);

struct type* infer_type(semantic_context_t* ctx, astnode_t* node);

bool check_type_compatibility(semantic_context_t* ctx, astnode_t* node, struct type* expected, struct type* actual);
// struct type* get_binary_op_result_type(semantic_context_t* ctx, enum op_code op, struct type* left, struct type* right);
// struct type* get_unary_op_result_type(semantic_context_t* ctx, enum op_code op, struct type* operand);

bool all_paths_return(astnode_t* node);
bool is_unreachable_code(astnode_t* node);

semantic_context_t* new_semantic_context(void)
{
    semantic_context_t* ctx = (semantic_context_t*)malloc(sizeof(semantic_context_t));
    if(!ctx) return NULL;
    
    ctx->symbols = new_symbol_table();
    if(!ctx->symbols){
        free(ctx);
        return NULL;
    }
    ctx->current_function = NULL;
    ctx->loop_depth = 0;
    
    ctx->errors = NULL;
    ctx->errors_count = 0;
    
    return ctx;
}

void new_semantic_error(semantic_context_t* ctx, report_t* err)
{
    if(!ctx || !err){
        if(err) free_report(err);
        return;
    }

    if(!ctx->errors){
        ctx->errors = (report_t**)malloc(sizeof(report_t*));
        if(!ctx->errors){
            free_report(err);
            return;
        }
        ctx->errors[0] = err;
        ctx->errors_count = 1;
        return;
    }

    report_t** new_errors = (report_t**)realloc(ctx->errors, (ctx->errors_count + 1) * sizeof(report_t*));
    if(!new_errors){
        free_report(err);
        return;
    }
    
    ctx->errors = new_errors;
    ctx->errors[ctx->errors_count] = err;
    ctx->errors_count++;
}

bool analyze_ast(semantic_context_t* ctx, astnode_t* root)
{
    if(!ctx || !root) return false;
    return check_node(ctx, root);
}

void free_semantic_context(semantic_context_t* ctx)
{
    if(!ctx) return;
    
    if(ctx->symbols){
        free_symbol_table(ctx->symbols);
    }

    if(ctx->errors){
        for(size_t i = 0; i < ctx->errors_count; ++i){
            if(ctx->errors[i]) free_report(ctx->errors[i]);
        }
        free(ctx->errors);
    }

    ctx->symbols = NULL;
    ctx->errors = NULL;
    ctx->errors_count = 0;

    free(ctx);
    ctx = NULL;
}

bool check_node(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node) return false;
    
    switch(node->type){
        case NODE_VAR:       return check_variable(ctx, node);
        case NODE_VAR_REF:   return check_var_ref(ctx, node);
        case NODE_LITERAL:   return check_literal(ctx, node);
        case NODE_BIN_OP:    return check_binary_op(ctx, node);
        case NODE_UNARY_OP:  return check_unary_op(ctx, node);
        case NODE_FUNC_CALL: return check_func_call(ctx, node);
        case NODE_BLOCK:     return check_block(ctx, node);
        case NODE_IF:        return check_if(ctx, node);
        case NODE_WHILE:     return check_while(ctx, node);
        case NODE_FOR:       return check_for(ctx, node);
        case NODE_RETURN:    return check_return(ctx, node);
        case NODE_BREAK:     return check_break(ctx, node);
        case NODE_CONTINUE:  return check_continue(ctx, node);
        case NODE_FUNC:      return check_function(ctx, node);
        case NODE_ARRAY:     return check_array(ctx, node);
        case NODE_STRUCT:    return check_struct(ctx, node);
        case NODE_UNION:     return check_union(ctx, node);
        case NODE_ENUM:      return check_enum(ctx, node);
        default:
            // semantic_warning(ctx, node, "Unimplemented node type in semantic analysis");
            // report* report = new_report(
            //     SEVERITY_WARNING, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_UNIMPLEMENTED_NODE,
            //     node->line, node->column, node->length, node->input
            // );
            return true;
    }
}

bool check_function(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_FUNC) return false;
    
    struct node_func* func = node->func_decl;
    if(!func || !func->name) return false;
    
    // check if function already exists
    if(is_scope_symbol_exist(ctx->symbols, func->name)){
        // semantic_error(ctx, node, "Function '%s' already defined", func->name);
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_FUNCTION_ALREADY_DECLARED,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // create function type
    struct type* return_type = datatype_to_type(func->return_type);
    struct type* func_type = new_type_function(return_type, NULL, func->param_count);
    
    // add function to symbol table
    symbol_t* func_sym = define_symbol(ctx->symbols, func->name, SYMBOL_FUNC, func_type, node);
    if(!func_sym){
        // semantic_error(ctx, node, "Failed to define function '%s'", func->name);
        // report* err = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_ALREADY_DECLARED,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // create new scope for function body
    push_scope(ctx->symbols, SCOPE_FUNCTION, node);
    symbol_t* prev_func = ctx->current_function;
    ctx->current_function = func_sym;
    
    // add parameters to function scope
    for(size_t i = 0; i < func->param_count; i++){
        if(!check_node(ctx, func->params[i])){
            pop_scope(ctx->symbols);
            ctx->current_function = prev_func;
            return false;
        }
    }
    
    // check function body
    bool success = true;
    if(func->body){
        success = check_node(ctx, func->body);
    }
    
    // TODO: Check that all paths return if return type is not void
    
    pop_scope(ctx->symbols);
    ctx->current_function = prev_func;
    return success;
}

bool check_variable(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_VAR) return false;
    
    struct node_var* var = node->var_decl;
    if(!var || !var->name) return false;
    
    if(is_scope_symbol_exist(ctx->symbols, var->name)){
        // semantic_error(ctx, node, "Variable '%s' must have either a type annotation or initializer", var->name);
        // report* err = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_ALREADY_DECLARED,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    struct type* var_type = NULL; // determine
    
    if(var->dtype != DT_VOID){
        var_type = datatype_to_type(var->dtype); // explicit type annotation
    }
    else if(var->value){
        // type inference from initializer
        if(!check_node(ctx, var->value)) return false;
        var_type = infer_type(ctx, var->value);
    }
    else {
        // no type and no initializer
        // semantic_error(ctx, node, "Variable '%s' must have either a type annotation or initializer", var->name);
        // report* err = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER,
        //     node->line, node->column, node->length, node->input
        // )
        return false;
    }
    
    if(!var_type || var_type == type_error){
        // semantic_error(ctx, node, "Cannot determine type for variable '%s'", var->name);
        // report* err = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // check type compatibility if both annotation and initializer exist
    if(var->dtype != DT_VOID && var->value){
        struct type* init_type = infer_type(ctx, var->value);
        if(!check_type_compatibility(ctx, node, var_type, init_type)){
            // semantic_error(ctx, node, "Type mismatch: cannot assign %s to %s", type_to_string(init_type), type_to_string(var_type));
            // report* err = new_report(
            //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_TYPE_MISMATCH,
            //     node->line, node->column, node->length, node->input
            // );
            return false;
        }
    }
    
    // Determine symbol kind based on modifier
    enum symbol_kind kind = SYMBOL_VAR;
    if(var->modif == MOD_CONST || var->modif == MOD_FINAL){
        kind = SYMBOL_CONST;
    }
    
    // add to symbol table
    symbol_t* sym = define_symbol(ctx->symbols, var->name, kind, var_type, node);
    if(!sym){
        // semantic_error(ctx, node, "Failed to define variable '%s'", var->name);
        // report* err = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_PARSER, SEMANTIC_ERROR_VARIABLE_NO_TYPE_OR_INITIALIZER,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // mark as initialized if has value
    if(var->value){
        sym->flags |= SYM_FLAG_ASSIGNED;
    }
    
    return true;
}

bool check_block(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_BLOCK) return false;
    
    push_scope(ctx->symbols, SCOPE_BLOCK, node); // create new scope for block
    
    bool success = true;
    for(size_t i = 0; i < node->block.count; i++){
        if(!check_node(ctx, node->block.statements[i])){
            success = false;
            // TODO: continue checking other statements
        }
    }
    
    pop_scope(ctx->symbols);
    return success;
}

bool check_if(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_IF) return false;
    
    if(!check_node(ctx, node->if_stmt->condition)) return false; // check condition
    
    // check branches
    bool success = true;
    if(node->if_stmt->then_block)  success = check_node(ctx, node->if_stmt->then_block)  && success;
    if(node->if_stmt->else_block)  success = check_node(ctx, node->if_stmt->else_block)  && success;
    if(node->if_stmt->elif_blocks) success = check_node(ctx, node->if_stmt->elif_blocks) && success;
    
    return success;
}

bool check_while(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_WHILE) return false;
    
    ctx->loop_depth++;
    
    bool success = true;
    if(node->while_loop->condition) success = check_node(ctx, node->while_loop->condition) && success;
    if(node->while_loop->body) success = check_node(ctx, node->while_loop->body) && success;
    
    ctx->loop_depth--;
    return success;
}

bool check_for(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_FOR) return false;
    
    push_scope(ctx->symbols, SCOPE_BLOCK, node);
    ctx->loop_depth++;
    
    bool success = true;
    if(node->for_loop->init) success = check_node(ctx, node->for_loop->init) && success;
    if(node->for_loop->condition) success = check_node(ctx, node->for_loop->condition) && success;
    if(node->for_loop->update) success = check_node(ctx, node->for_loop->update) && success;
    if(node->for_loop->body) success = check_node(ctx, node->for_loop->body) && success;
    
    ctx->loop_depth--;
    pop_scope(ctx->symbols);
    return success;
}

bool check_return(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_RETURN) return false;
    
    if(!ctx->current_function){
        // semantic_error(ctx, node, "Return statement outside function");
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_RETURN_OUTSIDE_FUNCTION,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    if(node->return_stmt.body){
        return check_node(ctx, node->return_stmt.body);
    }
    
    return true;
}

bool check_break(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node) return false;
    
    if(ctx->loop_depth == 0){
        // semantic_error(ctx, node, "Break statement outside loop");
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_BREAK_OUTSIDE_LOOP,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    return true;
}

bool check_continue(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node) return false;
    
    if(ctx->loop_depth == 0){
        // semantic_error(ctx, node, "Continue statement outside loop");
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_CONTINUE_OUTSIDE_LOOP,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    return true;
}

bool check_expression(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_EXPR) return false;
    // TODO: Implement expression checking
    return true;
}

bool check_binary_op(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_BIN_OP) return false;
    
    // check both operands
    if(!check_node(ctx, node->bin_op.left)) return false;
    if(!check_node(ctx, node->bin_op.right)) return false;
    
    // infer types
    struct type* left_type = infer_type(ctx, node->bin_op.left);
    struct type* right_type = infer_type(ctx, node->bin_op.right);
    
    if(!left_type || left_type == type_error || !right_type || right_type == type_error){
        return false;
    }
    
    // check type compatibility for operation
    // enum op_code op = node->bin_op.code;
    // (void)op;  // TODO: use for operator-specific checks
    
    if(!types_compatible(left_type, right_type)){
        // semantic_error(ctx, node, "Type mismatch in binary operation: %s and %s", type_to_string(left_type), type_to_string(right_type));
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_TYPE_MISMATCH,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    return true;
}

bool check_unary_op(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_UNARY_OP) return false;
    
    return check_node(ctx, node->unary_op.right);
}

bool check_func_call(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_FUNC_CALL) return false;
    
    // Lookup function
    symbol_t* func_sym = lookup_symbol(ctx->symbols, node->func_call.name);
    if(!func_sym){
        // semantic_error(ctx, node, "Undefined function '%s'", node->func_call.name);
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_UNDECLARED_FUNCTION,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    if(func_sym->kind != SYMBOL_FUNC){
        // semantic_error(ctx, node, "'%s' is not a function", node->func_call.name);
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_NOT_A_FUNCTION,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // Check arguments
    for(size_t i = 0; i < node->func_call.arg_count; i++){
        if(!check_node(ctx, node->func_call.args[i])) return false;
    }
    
    // TODO: Check argument count and types match parameters
    
    func_sym->flags |= SYM_FLAG_USED;
    return true;
}

bool check_var_ref(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_VAR_REF) return false;
    
    const char* name = node->var_ref.name;
    if(!name) return false;
    
    // lookup variable
    symbol_t* sym = lookup_symbol(ctx->symbols, name);
    if(!sym){
        // semantic_error(ctx, node, "Undefined variable '%s'", name);
        // report* report = new_report(
        //     SEVERITY_ERROR, ERROR_TYPE_SEMANTIC, SEMANTIC_ERROR_UNDECLARED_VARIABLE,
        //     node->line, node->column, node->length, node->input
        // );
        return false;
    }
    
    // Mark as used
    sym->flags |= SYM_FLAG_USED;
    
    return true;
}

bool check_literal(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_LITERAL) return false;
    // Literals are always valid
    return true;
}

bool check_array(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_ARRAY) return false;
    
    // Check all elements
    for(size_t i = 0; i < node->array_decl->count; i++){
        if(!check_node(ctx, node->array_decl->elements[i])) return false;
    }
    
    return true;
}

bool check_struct(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_STRUCT) return false;
    // TODO: Implement struct checking
    return true;
}

bool check_union(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_UNION) return false;
    // TODO: Implement union checking
    return true;
}

bool check_enum(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node || node->type != NODE_ENUM) return false;
    // TODO: Implement enum checking
    return true;
}


struct type* infer_type(semantic_context_t* ctx, astnode_t* node)
{
    if(!ctx || !node) return type_error;
    
    switch(node->type){
        case NODE_LITERAL:
            switch(node->literal.type){
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
            
        case NODE_VAR_REF: {
            symbol_t* sym = lookup_symbol(ctx->symbols, node->var_ref.name);
            return sym ? sym->type : type_error;
        }
        
        // case NODE_BIN_OP:
        //     return get_binary_op_result_type(ctx, node->bin_op.code, infer_type(ctx, node->bin_op.left), infer_type(ctx, node->bin_op.right));
        
        // case NODE_UNARY_OP:
        //     return get_unary_op_result_type(ctx, node->unary_op.code, infer_type(ctx, node->unary_op.right));
        
        case NODE_FUNC_CALL: {
            symbol_t* func = lookup_symbol(ctx->symbols, node->func_call.name);
            if(func && func->type && func->type->kind == TYPE_FUNCTION){
                return func->type->func.return_type;
            }
            return type_error;
        }
        
        default:
            return type_unknown;
    }
}


bool check_type_compatibility(semantic_context_t* ctx, astnode_t* node, struct type* expected, struct type* actual)
{
    (void)ctx;
    (void)node;
    return types_compatible(expected, actual);
}

// struct type* get_binary_op_result_type(semantic_context_t* ctx, enum op_code op, struct type* left, struct type* right)
// {
//     (void)ctx;
//     (void)op;
    
//     if(!left || !right) return type_error;
    
//     // For now, simple rule: result is left type if compatible
//     if(types_compatible(left, right)){
//         return left;
//     }
    
//     return type_error;
// }

// struct type* get_unary_op_result_type(semantic_context_t* ctx, enum op_code op, struct type* operand)
// {
//     (void)ctx;
//     (void)op;
//     return operand;  // For now, result type = operand type
// }


bool all_paths_return(astnode_t* node)
{
    // TODO: Implement control flow analysis
    (void)node;
    return true;
}

bool is_unreachable_code(astnode_t* node)
{
    // TODO: Implement unreachable code detection
    (void)node;
    return false;
}
