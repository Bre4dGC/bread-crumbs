#include "compiler/frontend/lexer/tokens.h"
#include "compiler/frontend/ast.h"

node_t* new_node(arena_t* arena, enum node_kind kind)
{
    node_t* node = (node_t*)arena_alloc(arena, sizeof(node_t), alignof(node_t));
    if (!node) return NULL;

    node->kind = kind;
    node->loc = (location_t){1, 1};
    node->length = 0;

    switch (kind)
    {
        case NODE_BINOP:
            node->binop = (struct node_binop*)arena_alloc(arena, sizeof(struct node_binop), alignof(struct node_binop));
            if(!node->binop) return NULL;
            node->binop->left = NULL;
            node->binop->right = NULL;
            node->binop->operator = 0;
#ifdef DEBUG
            node->binop->lit = NULL;
#endif
            break;
        case NODE_UNARYOP:
            node->unaryop = (struct node_unaryop*)arena_alloc(arena, sizeof(struct node_unaryop), alignof(struct node_unaryop));
            if(!node->unaryop) return NULL;
            node->unaryop->right = NULL;
            node->unaryop->operator = 0;
            node->unaryop->is_postfix = false;
#ifdef DEBUG
            node->unaryop->lit = NULL;
#endif
            break;
        case NODE_VAR:
            node->var_decl = (struct node_var*)arena_alloc(arena, sizeof(struct node_var), alignof(struct node_var));
            if(!node->var_decl) return NULL;
            node->var_decl->modif = MOD_VAR;
            node->var_decl->dtype = DT_VOID;
            node->var_decl->value = NULL;
            node->var_decl->name = (string_t){0};
            break;
        case NODE_ASSIGN:
            node->var_assign = (struct node_var_assign*)arena_alloc(arena, sizeof(struct node_var_assign), alignof(struct node_var_assign));
            if(!node->var_assign) return NULL;
            node->var_assign->name = (string_t){0};
            node->var_assign->value = NULL;
            break;
        case NODE_REF:
            node->var_ref = (struct node_var_ref*)arena_alloc(arena, sizeof(struct node_var_ref), alignof(struct node_var_ref));
            if(!node->var_ref) return NULL;
            node->var_ref->name = (string_t){0};
            break;
        case NODE_BLOCK:
            node->block = (struct node_block*)arena_alloc(arena, sizeof(struct node_block), alignof(struct node_block));
            if(!node->block) return NULL;
            node->block->statement.elems = NULL;
            node->block->statement.count = 0;
            node->block->statement.capacity = 0;
            break;
        case NODE_CALL:
            node->func_call = (struct node_func_call*)arena_alloc(arena, sizeof(struct node_func_call), alignof(struct node_func_call));
            if(!node->func_call) return NULL;
            node->func_call->args.elems = NULL;
            node->func_call->args.count = 0;
            node->func_call->args.capacity = 0;
            break;
        case NODE_RETURN:
            node->return_stmt = (struct node_return*)arena_alloc(arena, sizeof(struct node_return), alignof(struct node_return));
            if(!node->return_stmt) return NULL;
            node->return_stmt->body = NULL;
            break;
        case NODE_LITERAL:
            node->lit = (struct node_literal*)arena_alloc(arena, sizeof(struct node_literal), alignof(struct node_literal));
            if(!node->lit) return NULL;
            node->lit->type = LIT_NULL;
            node->lit->value = (string_t){0};
            break;
        case NODE_FOR:
            node->for_stmt = (struct node_for*)arena_alloc(arena, sizeof(struct node_for), alignof(struct node_for));
            if(!node->for_stmt) return NULL;
            node->for_stmt->init = NULL;
            node->for_stmt->condition = NULL;
            node->for_stmt->update = NULL;
            node->for_stmt->body = NULL;
            break;
        case NODE_IF:
            node->if_stmt = (struct node_if*)arena_alloc(arena, sizeof(struct node_if), alignof(struct node_if));
            if(!node->if_stmt) return NULL;
            node->if_stmt->condition = NULL;
            node->if_stmt->then_block = NULL;
            node->if_stmt->elif_blocks = NULL;
            node->if_stmt->else_block = NULL;
            break;
        case NODE_WHILE:
            node->while_stmt = (struct node_while*)arena_alloc(arena, sizeof(struct node_while), alignof(struct node_while));
            if(!node->while_stmt) return NULL;
            node->while_stmt->condition = NULL;
            node->while_stmt->body = NULL;
            break;
        case NODE_PARAM:
            node->param_decl = (struct node_param*)arena_alloc(arena, sizeof(struct node_param), alignof(struct node_param));
            if(!node->param_decl) return NULL;
            node->param_decl->name = (string_t){0};
            break;
        case NODE_FUNC:
            node->func_decl = (struct node_func*)arena_alloc(arena, sizeof(struct node_func), alignof(struct node_func));
            if(!node->func_decl) return NULL;
            node->func_decl->return_type = DT_VOID;
            node->func_decl->body = NULL;
            node->func_decl->name = (string_t){0};
            node->func_decl->param_decl.count = 0;
            node->func_decl->param_decl.capacity = 4;
            node->func_decl->param_decl.elems = (node_t**)arena_alloc(arena, node->func_decl->param_decl.capacity * sizeof(node_t*), alignof(node_t*));
            if(!node->func_decl->param_decl.elems) return NULL;
            break;
        case NODE_ARRAY:
            node->array_decl = (struct node_array*)arena_alloc(arena, sizeof(struct node_array), alignof(struct node_array));
            if(!node->array_decl) return NULL;
            node->array_decl->elements = NULL;
            node->array_decl->count = 0;
            node->array_decl->capacity = 0;
            break;
        case NODE_MATCH:
            node->match_stmt = (struct node_match*)arena_alloc(arena, sizeof(struct node_match), alignof(struct node_match));
            if(!node->match_stmt) return NULL;
            node->match_stmt->target = NULL;
            node->match_stmt->block.elems = NULL;
            node->match_stmt->block.count = 0;
            node->match_stmt->block.capacity = 0;
            break;
        case NODE_CASE:
            node->case_stmt = (struct node_case*)arena_alloc(arena, sizeof(struct node_case), alignof(struct node_case));
            if(!node->case_stmt) return NULL;
            node->case_stmt->condition = NULL;
            node->case_stmt->body = NULL;
            break;
        case NODE_STRUCT:
            node->struct_decl = (struct node_struct*)arena_alloc(arena, sizeof(struct node_struct), alignof(struct node_struct));
            if(!node->struct_decl) return NULL;
            node->struct_decl->member.elems = NULL;
            node->struct_decl->member.count = 0;
            node->struct_decl->member.capacity = 0;
            node->struct_decl->name = (string_t){0};
            break;
        case NODE_ENUM:
            node->enum_decl = (struct node_enum*)arena_alloc(arena, sizeof(struct node_enum), alignof(struct node_enum));
            if(!node->enum_decl) return NULL;
            node->enum_decl->member.elems = NULL;
            node->enum_decl->member.count = 0;
            node->enum_decl->member.capacity = 0;
            node->enum_decl->name = (string_t){0};
            break;
        case NODE_TRAIT:
            node->trait_decl = (struct node_trait*)arena_alloc(arena, sizeof(struct node_trait), alignof(struct node_trait));
            if(!node->trait_decl) return NULL;
            node->trait_decl->body = NULL;
            node->trait_decl->name = (string_t){0};
            break;
        case NODE_IMPL:
            node->impl_decl = (struct node_impl*)arena_alloc(arena, sizeof(struct node_impl), alignof(struct node_impl));
            if(!node->impl_decl) return NULL;
            node->impl_decl->body = NULL;
            node->impl_decl->trait_name = (string_t){0};
            node->impl_decl->struct_name = (string_t){0};
            break;
        case NODE_TRYCATCH:
            node->trycatch_stmt = (struct node_trycatch*)arena_alloc(arena, sizeof(struct node_trycatch), alignof(struct node_trycatch));
            if(!node->trycatch_stmt) return NULL;
            node->trycatch_stmt->try_block = NULL;
            node->trycatch_stmt->catch_block = NULL;
            node->trycatch_stmt->finally_block = NULL;
            break;
        case NODE_TYPE:
            node->type_decl = (struct node_type*)arena_alloc(arena, sizeof(struct node_type), alignof(struct node_type));
            if(!node->type_decl) return NULL;
            node->type_decl->body = NULL;
            node->type_decl->name = (string_t){0};
            break;
        case NODE_IMPORT:
            node->import_decl = (struct node_import*)arena_alloc(arena, sizeof(struct node_import), alignof(struct node_import));
            if(!node->import_decl) return NULL;
            node->import_decl->count = 0;
            node->import_decl->capacity = 16;
            node->import_decl->modules = (string_t*)arena_alloc_array(arena, sizeof(string_t), node->import_decl->capacity, alignof(string_t));
            if(!node->import_decl->modules) return NULL;
            break;
        case NODE_MODULE:
            node->module_decl = (struct node_module*)arena_alloc(arena, sizeof(struct node_module), alignof(struct node_module));
            if(!node->module_decl) return NULL;
            node->module_decl->body = NULL;
            node->module_decl->name = (string_t){0};
            break;
        case NODE_NAMEOF:
        case NODE_TYPEOF:
            node->special_stmt = (struct node_special*)arena_alloc(arena, sizeof(struct node_special), alignof(struct node_special));
            if(!node->special_stmt) return NULL;
            node->special_stmt->type = 0;
            node->special_stmt->content = (string_t){0};
            break;
        default: break;
    }
    return node;
}

void free_ast(arena_t* root)
{
    if (!root) return;
    free_arena(root);
}
