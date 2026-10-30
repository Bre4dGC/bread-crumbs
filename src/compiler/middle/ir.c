#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler/middle/ir.h"
#include "compiler/frontend/semantic/symbol.h"
#include "compiler/frontend/lexer/tokens.h"
#include "compiler/context.h"
#include "core/ds/arena.h"
#include "core/ds/strings.h"

#include <llvm-c/Core.h>

ir_t* new_ir(void)
{
    ir_t* ir = calloc(1, sizeof(*ir));
    if(!ir) return NULL;
    ir->arena = new_arena(ARENA_PHASE_SIZE);
    if(!ir->arena){ free(ir); return NULL; }
    ir->next_reg = 1;
    return ir;
}

void free_ir(ir_t* ir)
{
    if(!ir) return;
    if(ir->arena) free_arena(ir->arena);
    free(ir);
}

ir_module_t* ir_module_create(ir_t* ir, const char* name)
{
    if(!ir) return NULL;
    ir->module = calloc(1, sizeof(*ir->module));
    if(!ir->module) return NULL;
    ir->module->name.data = (char*)name;
    ir->module->name.length = name ? strlen(name) : 0;
    ir->module->name.hash = 0;
    ir->module->first_func = NULL;
    ir->module->last_func = NULL;
    ir->module->global_count = 0;
    return ir->module;
}

static string_t ir_str(const char* s)
{
    string_t str;
    str.data = (char*)s;
    str.length = s ? strlen(s) : 0;
    str.hash = 0;
    return str;
}

ir_value_t* new_value(arena_t* arena, enum ir_val_kind kind, int type)
{
    ir_value_t* v = arena_alloc(arena, sizeof(*v), _Alignof(ir_value_t));
    if(!v) return NULL;
    v->kind = kind;
    v->type = type;
    v->name.data = NULL;
    v->name.length = 0;
    v->name.hash = 0;
    v->llvm_val = NULL;
    v->i = 0;
    v->next = NULL;
    return v;
}

ir_func_t* ir_func_create(ir_t* ir, ir_module_t* mod, const char* name,
                          int return_type, ir_value_t** params, size_t param_count)
{
    if(!ir || !mod) return NULL;
    ir_func_t* f = arena_alloc(ir->arena, sizeof(*f), _Alignof(ir_func_t));
    if(!f) return NULL;
    f->name = ir_str(name);
    f->return_type = return_type;
    f->params = params;
    f->param_count = param_count;
    f->block_count = 0;
    f->first_block = NULL;
    f->last_block = NULL;
    f->next = NULL;

    if(mod->last_func){
        mod->last_func->next = f;
    } else {
        mod->first_func = f;
    }
    mod->last_func = f;
    return f;
}

ir_block_t* ir_block_create(ir_t* ir, ir_func_t* func, const char* name)
{
    if(!ir || !func) return NULL;
    ir_block_t* b = arena_alloc(ir->arena, sizeof(*b), _Alignof(ir_block_t));
    if(!b) return NULL;
    b->name = ir_str(name);
    b->first = NULL;
    b->last = NULL;
    b->next = NULL;
    b->terminated = false;

    if(func->last_block){
        func->last_block->next = b;
    } else {
        func->first_block = b;
    }
    func->last_block = b;
    func->block_count++;
    return b;
}

void ir_global(ir_t* ir, const char* name, int type)
{
    (void)ir; (void)name; (void)type;
}

static ir_value_t* new_temp(ir_t* ir, int type)
{
    ir_value_t* v = new_value(ir->arena, IR_VAL_REG, type);
    if(!v) return NULL;
    char buf[64];
    snprintf(buf, sizeof(buf), "t%zu", ir->next_reg++);
    v->name.data = arena_alloc(ir->arena, strlen(buf) + 1, 1);
    if(v->name.data) memcpy((char*)v->name.data, buf, strlen(buf) + 1);
    v->name.length = strlen(buf);
    return v;
}

/* -------------------------------------------------------------------------- */
/* IR lowering from AST                                                       */
/* -------------------------------------------------------------------------- */

typedef struct ir_local {
    const char* name;
    int type;
    bool is_alloca;
    struct ir_local* next;
} ir_local_t;

typedef struct {
    ir_t* ir;
    ir_func_t* func;
    ir_block_t* block;
    symbol_table_t* symbols;
    ir_local_t* locals;
} ir_lower_t;

static void bind_local(ir_lower_t* l, const char* name, int type, bool is_alloca)
{
    if(!l || !name) return;
    ir_local_t* binding = arena_alloc(l->ir->arena, sizeof(*binding), _Alignof(ir_local_t));
    if(!binding) return;
    binding->name = name;
    binding->type = type;
    binding->is_alloca = is_alloca;
    binding->next = l->locals;
    l->locals = binding;
}

static ir_local_t* find_local(ir_lower_t* l, const char* name)
{
    if(!l || !name) return NULL;
    for(ir_local_t* binding = l->locals; binding; binding = binding->next){
        if(binding->name && strcmp(binding->name, name) == 0) return binding;
    }
    return NULL;
}

static ir_instr_t* new_instr(ir_t* ir, enum ir_instr_op op,
                             ir_value_t* result, ir_value_t** operands, size_t operand_count)
{
    ir_instr_t* i = arena_alloc(ir->arena, sizeof(*i), _Alignof(ir_instr_t));
    if(!i) return NULL;
    i->op = op;
    i->result = result;
    i->operand_count = operand_count;
    for(size_t k = 0; k < operand_count && k < IR_MAX_OPERANDS; k++){
        i->operands[k] = operands[k];
    }
    for(size_t k = operand_count; k < IR_MAX_OPERANDS; k++){
        i->operands[k] = NULL;
    }
    i->binop_op = IR_BINOP_ADD;
    i->next = NULL;
    return i;
}

static void emit_instr(ir_lower_t* l, ir_instr_t* i)
{
    if(!l->block->first){
        l->block->first = i;
        l->block->last = i;
    } else {
        l->block->last->next = i;
        l->block->last = i;
    }
}

static ir_value_t* emit_alloca(ir_lower_t* l, int type)
{
    ir_value_t* r = new_temp(l->ir, type);
    if(!r) return NULL;
    ir_instr_t* i = new_instr(l->ir, IR_ALLOCA, r, NULL, 0);
    if(!i) return NULL;
    emit_instr(l, i);
    return r;
}

static ir_value_t* emit_load(ir_lower_t* l, ir_value_t* ptr)
{
    ir_value_t* r = new_temp(l->ir, ptr->type);
    if(!r) return NULL;
    ir_value_t* ops[] = { ptr };
    ir_instr_t* i = new_instr(l->ir, IR_LOAD, r, ops, 1);
    if(!i) return NULL;
    emit_instr(l, i);
    return r;
}

static void emit_store(ir_lower_t* l, ir_value_t* ptr, ir_value_t* val)
{
    ir_value_t* ops[] = { ptr, val };
    ir_instr_t* i = new_instr(l->ir, IR_STORE, NULL, ops, 2);
    if(!i) return;
    emit_instr(l, i);
}

static ir_value_t* emit_binop(ir_lower_t* l, enum ir_binop_op bop,
                              ir_value_t* lhs, ir_value_t* rhs)
{
    int result_type = lhs->type;
    ir_value_t* r = new_temp(l->ir, result_type);
    if(!r) return NULL;
    ir_value_t* ops[] = { lhs, rhs };
    ir_instr_t* i = new_instr(l->ir, IR_BINOP, r, ops, 2);
    if(!i) return NULL;
    i->binop_op = bop;
    emit_instr(l, i);
    return r;
}

static void emit_ret(ir_lower_t* l, ir_value_t* val)
{
    ir_value_t* ops[] = { val };
    ir_instr_t* i = new_instr(l->ir, IR_RET, NULL, ops, 1);
    if(!i) return;
    emit_instr(l, i);
    l->block->terminated = true;
}

static void emit_ret_void(ir_lower_t* l)
{
    ir_instr_t* i = new_instr(l->ir, IR_RET_VOID, NULL, NULL, 0);
    if(!i) return;
    emit_instr(l, i);
    l->block->terminated = true;
}

/* -------------------------------------------------------------------------- */
/* expression lowering                                                        */
/* -------------------------------------------------------------------------- */

static ir_value_t* lower_expr(ir_lower_t* l, node_t* node);

static ir_value_t* emit_const_int_typed(ir_t* ir, int type, long val)
{
    ir_value_t* v = new_value(ir->arena, IR_VAL_CONST_INT, type);
    if(!v) return NULL;
    v->i = (int)val;
    return v;
}

static ir_value_t* emit_const_float(ir_t* ir, double val)
{
    ir_value_t* v = new_value(ir->arena, IR_VAL_CONST_FLOAT, DT_FLOAT);
    if(!v) return NULL;
    v->d = val;
    return v;
}

static ir_value_t* emit_const_bool(ir_t* ir, bool val)
{
    ir_value_t* v = new_value(ir->arena, IR_VAL_CONST_BOOL, DT_BOOL);
    if(!v) return NULL;
    v->b = val;
    return v;
}

static ir_value_t* emit_const_str(ir_t* ir, const char* val)
{
    ir_value_t* v = new_value(ir->arena, IR_VAL_CONST_STR, DT_STR);
    if(!v) return NULL;
    v->str = ir_str(val ? val : "");
    return v;
}

static long parse_int_literal(const node_t* node)
{
    const char* text = node->lit->value.data ? node->lit->value.data : "0";

    if(node->lit->type == LIT_BIN){
        long value = 0;
        const char* p = text;
        if(p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) p += 2;
        while(*p == '0' || *p == '1'){
            value = value * 2 + (*p - '0');
            p++;
        }
        return value;
    }

    return strtol(text, NULL, 0);
}

static ir_value_t* lower_literal(ir_lower_t* l, node_t* node)
{
    const char* text = node->lit->value.data ? node->lit->value.data : "";

    switch(node->lit->type){
        case LIT_NUMBER:
        case LIT_HEX:
        case LIT_BIN:
            return emit_const_int_typed(l->ir, DT_INT, parse_int_literal(node));
        case LIT_FLOAT:
            return emit_const_float(l->ir, atof(text));
        case LIT_TRUE:
            return emit_const_bool(l->ir, true);
        case LIT_FALSE:
            return emit_const_bool(l->ir, false);
        case LIT_STRING:
            return emit_const_str(l->ir, text);
        case LIT_CHAR:
            return emit_const_int_typed(l->ir, DT_CHAR, (long)text[0]);
        case LIT_NULL:
            return new_value(l->ir->arena, IR_VAL_CONST_NULL, DT_VOID);
        default:
            return emit_const_int_typed(l->ir, DT_INT, 0);
    }
}

/* Map the language level print/write/eprint builtins onto the runtime symbols. */
static const char* runtime_symbol_for(ir_t* ir, const char* name, int arg_type)
{
    if(strcmp(name, "read_line") == 0) return "brt_read_line";
    if(strcmp(name, "read_all") == 0)  return "brt_read_all";

    const char* prefix = NULL;
    if(strcmp(name, "print") == 0)       prefix = "brt_print";
    else if(strcmp(name, "write") == 0)  prefix = "brt_write";
    else if(strcmp(name, "eprint") == 0) prefix = "brt_eprint";
    else return NULL;

    const char* suffix = "";
    switch(arg_type){
        case DT_STR:     suffix = "";       break;
        case DT_BOOL:    suffix = "_bool";  break;
        case DT_CHAR:    suffix = "_char";  break;
        case DT_FLOAT:
        case DT_DECIMAL: suffix = "_float"; break;
        default:         suffix = "_int";   break;
    }

    char buffer[64];
    int written = snprintf(buffer, sizeof(buffer), "%s%s", prefix, suffix);
    if(written < 0 || (size_t)written >= sizeof(buffer)) return NULL;

    char* symbol = arena_alloc(ir->arena, (size_t)written + 1, 1);
    if(!symbol) return NULL;
    memcpy(symbol, buffer, (size_t)written + 1);
    return symbol;
}

static ir_value_t* lower_call(ir_lower_t* l, node_t* node)
{
    if(!node || !node->func_call || !node->func_call->name.data) return NULL;

    size_t arg_count = node->func_call->args.count;
    if(arg_count > IR_MAX_OPERANDS - 1) arg_count = IR_MAX_OPERANDS - 1;

    ir_value_t* args[IR_MAX_OPERANDS];
    for(size_t k = 0; k < arg_count; k++){
        args[k] = lower_expr(l, node->func_call->args.elems[k]);
    }

    symbol_t* symbol = l->symbols ? lookup_symbol(l->symbols, node->func_call->name.data) : NULL;
    int return_type = DT_VOID;
    if(symbol && symbol->type && symbol->type->kind == TYPE_FUNC){
        return_type = type_to_datatype(symbol->type->func.return_type);
    }

    int first_arg_type = (arg_count > 0 && args[0]) ? args[0]->type : DT_VOID;
    const char* target = runtime_symbol_for(l->ir, node->func_call->name.data, first_arg_type);
    if(!target) target = node->func_call->name.data;

    ir_value_t* function = new_value(l->ir->arena, IR_VAL_FUNC, DT_VOID);
    if(!function) return NULL;
    function->name = ir_str(target);

    ir_value_t* operands[IR_MAX_OPERANDS];
    operands[0] = function;
    for(size_t k = 0; k < arg_count; k++) operands[k + 1] = args[k];

    ir_value_t* result = NULL;
    if(return_type != DT_VOID){
        result = new_temp(l->ir, return_type);
        if(!result) return NULL;
    }

    ir_instr_t* instr = new_instr(l->ir, IR_CALL, result, operands, arg_count + 1);
    if(!instr) return NULL;
    emit_instr(l, instr);
    return result;
}

/* Assignment is parsed as a binary expression whose left side is a reference. */
static ir_value_t* lower_assign(ir_lower_t* l, node_t* node, enum category_operator op)
{
    node_t* target = node->binop->left;
    if(!target || target->kind != NODE_REFERENCE) return NULL;

    const char* name = target->var_ref->name.data;
    ir_local_t* local = find_local(l, name);

    /* Only locals backed by an alloca can be written back. */
    if(!local || !local->is_alloca) return NULL;

    ir_value_t* value = lower_expr(l, node->binop->right);
    if(!value) return NULL;

    ir_value_t* ref = new_value(l->ir->arena, IR_VAL_REG, local->type);
    if(!ref) return NULL;
    ref->name = target->var_ref->name;

    if(op != OPER_ASSIGN){
        enum ir_binop_op bop;
        switch(op){
            case OPER_ADD: bop = IR_BINOP_ADD; break;
            case OPER_SUB: bop = IR_BINOP_SUB; break;
            case OPER_MUL: bop = IR_BINOP_MUL; break;
            case OPER_DIV: bop = IR_BINOP_DIV; break;
            case OPER_MOD: bop = IR_BINOP_MOD; break;
            default: return NULL;
        }

        ir_value_t* current = emit_load(l, ref);
        if(!current) return NULL;

        value = emit_binop(l, bop, current, value);
        if(!value) return NULL;
    }

    emit_store(l, ref, value);
    return value;
}

static ir_value_t* lower_expr(ir_lower_t* l, node_t* node)
{
    if(!l || !node) return NULL;

    switch(node->kind){
        case NODE_LITERAL:
            return lower_literal(l, node);

        case NODE_REFERENCE: {
            const char* name = node->var_ref->name.data;
            if(!name) return NULL;

            ir_local_t* local = find_local(l, name);
            int type = local ? local->type : DT_INT;

            ir_value_t* reference = new_value(l->ir->arena, IR_VAL_REG, type);
            if(!reference) return NULL;
            reference->name = node->var_ref->name;

            if(local && local->is_alloca) return emit_load(l, reference);
            return reference;
        }

        case NODE_CALL:
            return lower_call(l, node);

        case NODE_BINOP: {
            const enum category_operator assign_op = node->binop->operator;
            if(assign_op == OPER_ASSIGN || assign_op == OPER_ADD || assign_op == OPER_SUB ||
               assign_op == OPER_MUL || assign_op == OPER_DIV || assign_op == OPER_MOD){
                return lower_assign(l, node, assign_op);
            }

            ir_value_t* lhs = lower_expr(l, node->binop->left);
            ir_value_t* rhs = lower_expr(l, node->binop->right);
            if(!lhs || !rhs) return NULL;

            enum ir_binop_op op;
            switch(node->binop->operator){
                case OPER_PLUS:     op = IR_BINOP_ADD;  break;
                case OPER_MINUS:    op = IR_BINOP_SUB;  break;
                case OPER_ASTERISK: op = IR_BINOP_MUL;  break;
                case OPER_SLASH:    op = IR_BINOP_DIV;  break;
                case OPER_PERCENT:  op = IR_BINOP_MOD;  break;
                case OPER_EQ:       op = IR_BINOP_EQ;   break;
                case OPER_NEQ:      op = IR_BINOP_NEQ;  break;
                case OPER_LANGLE:   op = IR_BINOP_LT;   break;
                case OPER_RANGLE:   op = IR_BINOP_GT;   break;
                case OPER_LTE:      op = IR_BINOP_LTE;  break;
                case OPER_GTE:      op = IR_BINOP_GTE;  break;
                case OPER_AND:      op = IR_BINOP_AND;  break;
                case OPER_OR:       op = IR_BINOP_OR;   break;
                default:            return NULL;
            }
            return emit_binop(l, op, lhs, rhs);
        }

        default:
            return NULL;
    }
}

/* Lower a block statement into IR instructions */
static void lower_block(ir_lower_t* l, node_t* block_node)
{
    if(!block_node || block_node->kind != NODE_BLOCK) return;

    for(size_t k = 0; k < block_node->block->statement.count; k++){
        node_t* stmt = block_node->block->statement.elems[k];
        if(!stmt) continue;

        switch(stmt->kind){
        case NODE_VARIABLE: {
            struct node_variable* vd = stmt->var_decl;
            if(!vd) break;

            /* Evaluate the initializer first so its type can be inferred. */
            ir_value_t* value = vd->value ? lower_expr(l, vd->value) : NULL;

            int type = vd->dtype;
            if(type == DT_VOID && value) type = value->type;
            if(type == DT_VOID) type = DT_INT;

            ir_value_t* alloc = emit_alloca(l, type);
            if(!alloc) break;
            alloc->name = vd->name;

            if(vd->name.data) bind_local(l, vd->name.data, type, true);

            if(value) emit_store(l, alloc, value);
            break;
        }
        case NODE_RETURN: {
            struct node_return* rd = stmt->return_stmt;
            if(!rd || !rd->body){
                if(l->func && l->func->return_type != DT_VOID){
                    emit_ret(l, emit_const_int_typed(l->ir, DT_INT, 0));
                }
                else {
                    emit_ret_void(l);
                }
                break;
            }

            ir_value_t* value = lower_expr(l, rd->body);
            if(value){
                emit_ret(l, value);
            }
            else if(l->func && l->func->return_type != DT_VOID){
                emit_ret(l, emit_const_int_typed(l->ir, DT_INT, 0));
            }
            else {
                emit_ret_void(l);
            }
            break;
        }
        case NODE_CALL:
        case NODE_BINOP:
        case NODE_UNARYOP:
        case NODE_REFERENCE:
            (void)lower_expr(l, stmt);
            break;
        case NODE_BLOCK:
            /* Recursively lower nested blocks */
            lower_block(l, stmt);
            break;
        default:
            break;
        }
    }
}

static ir_func_t* find_ir_func(ir_t* ir, const char* name)
{
    if(!ir || !ir->module || !name) return NULL;
    for(ir_func_t* f = ir->module->first_func; f; f = f->next){
        if(f->name.data && strcmp(f->name.data, name) == 0) return f;
    }
    return NULL;
}

static ir_func_t* create_ir_func(ir_t* ir, node_t* func_node)
{
    struct node_func* fd = func_node->func_decl;

    ir_value_t** params = NULL;
    if(fd->param_decl.count > 0){
        params = arena_alloc_array(ir->arena, sizeof(ir_value_t*),
                                   fd->param_decl.count, _Alignof(ir_value_t*));
        if(!params) return NULL;
        for(size_t k = 0; k < fd->param_decl.count; k++){
            struct node_param* prm = fd->param_decl.elems[k]->param_decl;
            params[k] = new_value(ir->arena, IR_VAL_PARAM, prm->dtype);
            if(params[k]){
                params[k]->name = prm->name;
            }
        }
    }

    int return_type = fd->return_type;
    /* C entry points must return int. */
    if(fd->name.data && strcmp(fd->name.data, "main") == 0 && return_type == DT_VOID){
        return_type = DT_INT;
    }

    return ir_func_create(ir, ir->module, fd->name.data,
                          return_type, params, fd->param_decl.count);
}

static void lower_func(ir_t* ir, node_t* func_node, compiler_context_t* ctx, node_t* top_block)
{
    if(!ir || !func_node || func_node->kind != NODE_FUNC) return;

    struct node_func* fd = func_node->func_decl;
    if(!fd || !fd->name.data) return;

    ir_func_t* f = find_ir_func(ir, fd->name.data);
    if(!f){
        f = create_ir_func(ir, func_node);
        if(!f) return;
    }

    /* Create entry block and lower body */
    ir_block_t* entry = ir_block_create(ir, f, "entry");
    if(!entry) return;

    ir_lower_t l;
    l.ir = ir;
    l.func = f;
    l.block = entry;
    l.symbols = ctx->symbols;
    l.locals = NULL;

    /* Bind parameters so references resolve to the incoming values. */
    for(size_t k = 0; k < f->param_count; k++){
        ir_value_t* param = f->params[k];
        if(param && param->name.data){
            bind_local(&l, param->name.data, param->type, false);
        }
    }

    /* Top-level statements execute before the body of main. */
    if(top_block && top_block->kind == NODE_BLOCK && strcmp(f->name.data, "main") == 0){
        for(size_t k = 0; k < top_block->block->statement.count; k++){
            node_t* stmt = top_block->block->statement.elems[k];
            if(stmt && stmt->kind == NODE_CALL){
                (void)lower_expr(&l, stmt);
            }
        }
    }

    lower_block(&l, fd->body);

    /* Ensure terminator */
    if(!entry->terminated){
        if(f->return_type == DT_VOID) emit_ret_void(&l);
        else emit_ret(&l, emit_const_int_typed(ir, DT_INT, 0));
    }
}

/* -------------------------------------------------------------------------- */
/* Build                                                                      */
/* -------------------------------------------------------------------------- */

ir_t* ir_build(ast_t* ast, symbol_table_t* symbols, compiler_context_t* ctx)
{
    (void)symbols;
    ir_t* ir = ctx->ir;
    if(!ir){
        ir = new_ir();
        if(!ir) return NULL;
    }
    ir_module_create(ir, "main");

    /* Iterate over top-level statements in the program block */
    node_t* program_block = ast->nodes;
    if(!program_block || program_block->kind != NODE_BLOCK){
        return ir;
    }

    /* Pass 1: register functions and globals */
    for(size_t k = 0; k < program_block->block->statement.count; k++){
        node_t* node = program_block->block->statement.elems[k];
        switch(node->kind){
        case NODE_FUNC: {
            if(!create_ir_func(ir, node)) return ir;
            break;
        }
        case NODE_VARIABLE: {
            struct node_variable* vd = node->var_decl;
            if(vd && (vd->name.data)){
                symbol_t* sym = lookup_symbol(symbols, vd->name.data);
                if(sym && (sym->flags & SYM_FLAG_GLOBAL)){
                    ir_global(ir, vd->name.data, vd->dtype);
                }
            }
            break;
        }
        default: break;
        }
    }

    /* Pass 2: lower function bodies */
    for(size_t k = 0; k < program_block->block->statement.count; k++){
        node_t* node = program_block->block->statement.elems[k];
        if(node && node->kind == NODE_FUNC){
            lower_func(ir, node, ctx, program_block);
        }
    }

    return ir;
}
