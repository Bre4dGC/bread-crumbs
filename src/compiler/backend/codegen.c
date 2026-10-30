#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>

#include "core/ds/arena.h"
#include "core/ds/hashmap.h"

#include "compiler/backend/codegen.h"
#include "compiler/frontend/semantic/types.h"
#include "compiler/frontend/lexer/tokens.h"

#define CODEGEN_MODULE_NAME "breadcrumbs"

static LLVMTypeRef llvm_type_of_dt(codegen_t* cg, int dt)
{
    (void)cg;
    switch(dt){
        case DT_VOID:  return LLVMVoidTypeInContext(cg->ctx);
        case DT_BOOL:  return LLVMInt1TypeInContext(cg->ctx);
        case DT_BYTE:
        case DT_CHAR:  return LLVMInt8TypeInContext(cg->ctx);
        case DT_SHORT: return LLVMInt16TypeInContext(cg->ctx);
        case DT_INT:
        case DT_UINT:  return LLVMInt32TypeInContext(cg->ctx);
        case DT_LONG:
        case DT_ULONG: return LLVMInt64TypeInContext(cg->ctx);
        case DT_FLOAT:
        case DT_DECIMAL: return LLVMDoubleTypeInContext(cg->ctx);
        case DT_STR:   return LLVMPointerType(LLVMInt8TypeInContext(cg->ctx), 0);
        default:       return LLVMInt8TypeInContext(cg->ctx);
    }
}

static bool is_float_type(int dt)
{
    return dt == DT_FLOAT || dt == DT_DECIMAL;
}

/* -------------------------------------------------------------------------- */
/* runtime declarations                                                       */
/* -------------------------------------------------------------------------- */

static void declare_runtime_fn(codegen_t* cg, const char* name, LLVMTypeRef ret,
                               LLVMTypeRef* params, unsigned count)
{
    if(LLVMGetNamedFunction(cg->module, name)) return;
    LLVMTypeRef type = LLVMFunctionType(ret, params, count, false);
    LLVMAddFunction(cg->module, name, type);
}

static void declare_runtime(codegen_t* cg)
{
    LLVMTypeRef void_ty = LLVMVoidTypeInContext(cg->ctx);
    LLVMTypeRef i1 = LLVMInt1TypeInContext(cg->ctx);
    LLVMTypeRef i8 = LLVMInt8TypeInContext(cg->ctx);
    LLVMTypeRef i32 = LLVMInt32TypeInContext(cg->ctx);
    LLVMTypeRef f64 = LLVMDoubleTypeInContext(cg->ctx);
    LLVMTypeRef i8ptr = LLVMPointerType(i8, 0);

    LLVMTypeRef str_arg[] = { i8ptr };
    LLVMTypeRef int_arg[] = { i32 };
    LLVMTypeRef float_arg[] = { f64 };
    LLVMTypeRef bool_arg[] = { i1 };
    LLVMTypeRef char_arg[] = { i8 };

    declare_runtime_fn(cg, "brt_print", void_ty, str_arg, 1);
    declare_runtime_fn(cg, "brt_print_int", void_ty, int_arg, 1);
    declare_runtime_fn(cg, "brt_print_float", void_ty, float_arg, 1);
    declare_runtime_fn(cg, "brt_print_bool", void_ty, bool_arg, 1);
    declare_runtime_fn(cg, "brt_print_char", void_ty, char_arg, 1);

    declare_runtime_fn(cg, "brt_write", void_ty, str_arg, 1);
    declare_runtime_fn(cg, "brt_write_int", void_ty, int_arg, 1);
    declare_runtime_fn(cg, "brt_write_float", void_ty, float_arg, 1);
    declare_runtime_fn(cg, "brt_write_bool", void_ty, bool_arg, 1);
    declare_runtime_fn(cg, "brt_write_char", void_ty, char_arg, 1);

    declare_runtime_fn(cg, "brt_eprint", void_ty, str_arg, 1);
    declare_runtime_fn(cg, "brt_eprint_int", void_ty, int_arg, 1);
    declare_runtime_fn(cg, "brt_eprint_float", void_ty, float_arg, 1);
    declare_runtime_fn(cg, "brt_eprint_bool", void_ty, bool_arg, 1);
    declare_runtime_fn(cg, "brt_eprint_char", void_ty, char_arg, 1);

    declare_runtime_fn(cg, "brt_flush", void_ty, NULL, 0);
    declare_runtime_fn(cg, "brt_read_line", i8ptr, NULL, 0);
    declare_runtime_fn(cg, "brt_read_all", i8ptr, NULL, 0);
    declare_runtime_fn(cg, "brt_free", void_ty, str_arg, 1);
}

/* -------------------------------------------------------------------------- */
/* value lowering                                                             */
/* -------------------------------------------------------------------------- */

static LLVMValueRef value_to_llvm(codegen_t* cg, ir_value_t* v)
{
    if(!v) return LLVMConstNull(LLVMInt8TypeInContext(cg->ctx));

    if(v->llvm_val) return v->llvm_val;

    switch(v->kind){
        case IR_VAL_CONST_INT: {
            LLVMTypeRef ty = llvm_type_of_dt(cg, v->type);
            LLVMValueRef val = LLVMConstInt(ty, v->i, false);
            v->llvm_val = val;
            return val;
        }
        case IR_VAL_CONST_FLOAT:
            v->llvm_val = LLVMConstReal(LLVMDoubleTypeInContext(cg->ctx), v->d);
            return v->llvm_val;
        case IR_VAL_CONST_BOOL:
            v->llvm_val = LLVMConstInt(LLVMInt1TypeInContext(cg->ctx), v->b ? 1 : 0, false);
            return v->llvm_val;
        case IR_VAL_CONST_STR:
            v->llvm_val = LLVMBuildGlobalStringPtr(cg->builder, v->str.data ? v->str.data : "", "str");
            return v->llvm_val;
        case IR_VAL_CONST_NULL:
            v->llvm_val = LLVMConstNull(LLVMInt8TypeInContext(cg->ctx));
            return v->llvm_val;
        case IR_VAL_REG:
        case IR_VAL_PARAM:
        case IR_VAL_ALLOCA:
        case IR_VAL_GLOBAL: {
            LLVMValueRef found = (LLVMValueRef)hm_lookup(cg->named_values,
                                                         v->name.data ? v->name.data : "");
            if(found){
                v->llvm_val = found;
                return found;
            }
            if(v->kind == IR_VAL_GLOBAL){
                LLVMValueRef g = LLVMGetNamedGlobal(cg->module,
                                                    v->name.data ? v->name.data : "");
                if(g){
                    v->llvm_val = g;
                    return g;
                }
            }
            return LLVMConstNull(LLVMInt8TypeInContext(cg->ctx));
        }
        case IR_VAL_FUNC:
            return LLVMGetNamedFunction(cg->module,
                                        v->name.data ? v->name.data : "");
        default:
            return LLVMConstNull(LLVMInt8TypeInContext(cg->ctx));
    }
}

static void bind_value(codegen_t* cg, ir_value_t* v, LLVMValueRef llvm_val)
{
    if(!v || !v->name.data) return;
    hm_insert(cg->named_values, v->name.data, llvm_val);
    v->llvm_val = llvm_val;
}

/* -------------------------------------------------------------------------- */
/* block resolution                                                           */
/* -------------------------------------------------------------------------- */

typedef struct {
    ir_block_t* ir_block;
    LLVMBasicBlockRef llvm_block;
} block_map_t;

static LLVMBasicBlockRef resolve_bb(block_map_t* map, size_t count, ir_block_t* target)
{
    for(size_t k = 0; k < count; ++k){
        if(map[k].ir_block == target) return map[k].llvm_block;
    }
    return NULL;
}

/* -------------------------------------------------------------------------- */
/* instruction emission                                                       */
/* -------------------------------------------------------------------------- */

static void emit_instr(codegen_t* cg, block_map_t* map, size_t nblocks, ir_instr_t* i)
{
    switch(i->op){
        case IR_CONST: {
            LLVMValueRef val = value_to_llvm(cg, i->operands[0]);
            bind_value(cg, i->result, val);
            break;
        }
        case IR_ALLOCA: {
            LLVMTypeRef ty = llvm_type_of_dt(cg, i->result->type);
            LLVMValueRef alloca = LLVMBuildAlloca(cg->builder, ty,
                                                   i->result->name.data ? i->result->name.data : "alloca");
            bind_value(cg, i->result, alloca);
            break;
        }
        case IR_LOAD: {
            LLVMValueRef ptr = value_to_llvm(cg, i->operands[0]);
            if(!ptr){ bind_value(cg, i->result, ptr); break; }
            LLVMTypeRef ptr_ty = LLVMTypeOf(ptr);
            if(LLVMGetTypeKind(ptr_ty) != LLVMPointerTypeKind){ bind_value(cg, i->result, ptr); break; }
            /* In LLVM 15+, pointers are opaque; use the stored DT type for the element type */
            LLVMTypeRef ty = llvm_type_of_dt(cg, i->operands[0]->type);
            LLVMValueRef val = LLVMBuildLoad2(cg->builder, ty, ptr,
                                              i->result->name.data ? i->result->name.data : "load");
            bind_value(cg, i->result, val);
            break;
        }
        case IR_STORE: {
            LLVMValueRef ptr = value_to_llvm(cg, i->operands[0]);
            LLVMValueRef val = value_to_llvm(cg, i->operands[1]);
            LLVMBuildStore(cg->builder, val, ptr);
            break;
        }
        case IR_BINOP: {
            LLVMValueRef lhs = value_to_llvm(cg, i->operands[0]);
            LLVMValueRef rhs = value_to_llvm(cg, i->operands[1]);
            bool flt = is_float_type(i->result->type);
            const char* name = i->result->name.data ? i->result->name.data : "binop";
            switch((enum ir_binop_op)i->binop_op){
                case IR_BINOP_ADD:
                    bind_value(cg, i->result, flt ? LLVMBuildFAdd(cg->builder, lhs, rhs, name)
                                                  : LLVMBuildAdd(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_SUB:
                    bind_value(cg, i->result, flt ? LLVMBuildFSub(cg->builder, lhs, rhs, name)
                                                  : LLVMBuildSub(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_MUL:
                    bind_value(cg, i->result, flt ? LLVMBuildFMul(cg->builder, lhs, rhs, name)
                                                  : LLVMBuildMul(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_DIV:
                    bind_value(cg, i->result, flt ? LLVMBuildFDiv(cg->builder, lhs, rhs, name)
                                                  : LLVMBuildSDiv(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_MOD:
                    bind_value(cg, i->result, LLVMBuildSRem(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_EQ:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealOEQ, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntEQ, lhs, rhs, name));
                    break;
                case IR_BINOP_NEQ:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealONE, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntNE, lhs, rhs, name));
                    break;
                case IR_BINOP_LT:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealOLT, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntSLT, lhs, rhs, name));
                    break;
                case IR_BINOP_LTE:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealOLE, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntSLE, lhs, rhs, name));
                    break;
                case IR_BINOP_GT:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealOGT, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntSGT, lhs, rhs, name));
                    break;
                case IR_BINOP_GTE:
                    bind_value(cg, i->result, flt ? LLVMBuildFCmp(cg->builder, LLVMRealOGE, lhs, rhs, name)
                                                  : LLVMBuildICmp(cg->builder, LLVMIntSGE, lhs, rhs, name));
                    break;
                case IR_BINOP_AND:
                    bind_value(cg, i->result, LLVMBuildAnd(cg->builder, lhs, rhs, name));
                    break;
                case IR_BINOP_OR:
                    bind_value(cg, i->result, LLVMBuildOr(cg->builder, lhs, rhs, name));
                    break;
            }
            break;
        }
        case IR_UNOP: {
            LLVMValueRef v = value_to_llvm(cg, i->operands[0]);
            bool flt = is_float_type(i->result->type);
            const char* name = i->result->name.data ? i->result->name.data : "unop";
            switch((enum ir_unop_op)i->unop_op){
                case IR_UNOP_NEG:
                    bind_value(cg, i->result, flt ? LLVMBuildFNeg(cg->builder, v, name)
                                                  : LLVMBuildNeg(cg->builder, v, name));
                    break;
                case IR_UNOP_NOT:
                    bind_value(cg, i->result, LLVMBuildNot(cg->builder, v, name));
                    break;
            }
            break;
        }
        case IR_CALL: {
            LLVMValueRef fn = value_to_llvm(cg, i->operands[0]);
            if(!fn) break;
            LLVMTypeRef fnty = LLVMGlobalGetValueType(fn);
            LLVMValueRef args[IR_MAX_OPERANDS];
            unsigned nargs = 0;
            for(size_t k = 1; k < i->operand_count && nargs < IR_MAX_OPERANDS; ++k, ++nargs){
                args[nargs] = value_to_llvm(cg, i->operands[k]);
            }
            LLVMValueRef call = LLVMBuildCall2(cg->builder, fnty, fn, args, nargs,
                                               (i->result && i->result->name.data)
                                                   ? i->result->name.data : "");
            if(i->result) bind_value(cg, i->result, call);
            break;
        }
        case IR_RET: {
            LLVMValueRef val = value_to_llvm(cg, i->operands[0]);
            LLVMBuildRet(cg->builder, val);
            break;
        }
        case IR_RET_VOID:
            LLVMBuildRetVoid(cg->builder);
            break;
        case IR_BR: {
            LLVMBasicBlockRef dest = resolve_bb(map, nblocks, i->dest_block);
            if(!dest) dest = LLVMGetInsertBlock(cg->builder);
            LLVMBuildBr(cg->builder, dest);
            break;
        }
        case IR_COND_BR: {
            LLVMValueRef cond = value_to_llvm(cg, i->operands[0]);
            LLVMBasicBlockRef then_bb = resolve_bb(map, nblocks, i->cond_br.then_block);
            LLVMBasicBlockRef else_bb = resolve_bb(map, nblocks, i->cond_br.else_block);
            LLVMBasicBlockRef cur = LLVMGetInsertBlock(cg->builder);
            LLVMBuildCondBr(cg->builder, cond, then_bb ? then_bb : cur, else_bb ? else_bb : cur);
            break;
        }
        default:
            break;
    }
}

/* -------------------------------------------------------------------------- */
/* emit                                                                       */
/* -------------------------------------------------------------------------- */

static bool emit_function(codegen_t* cg, ir_func_t* f)
{
    LLVMValueRef fn = LLVMGetNamedFunction(cg->module,
                                           f->name.data ? f->name.data : "anon");
    if(!fn) return false;

    size_t n = f->param_count;

    /* collect blocks in order */
    size_t block_count = f->block_count;
    ir_block_t** blocks = malloc(sizeof(ir_block_t*) * (block_count ? block_count : 1));
    LLVMBasicBlockRef* llvm_blocks = malloc(sizeof(LLVMBasicBlockRef) * (block_count ? block_count : 1));
    block_map_t* map = malloc(sizeof(block_map_t) * (block_count ? block_count : 1));
    if(!blocks || !llvm_blocks || !map){
        free(blocks); free(llvm_blocks); free(map);
        return false;
    }

    size_t idx = 0;
    for(ir_block_t* b = f->first_block; b; b = b->next){
        blocks[idx] = b;
        llvm_blocks[idx] = LLVMAppendBasicBlock(fn, b->name.data ? b->name.data : "bb");
        map[idx].ir_block = b;
        map[idx].llvm_block = llvm_blocks[idx];
        ++idx;
    }

    if(cg->named_values) free_hashmap(cg->named_values);
    cg->named_values = new_hashmap();

    /* bind param names to LLVM param values; allocas are created by the
     * IR_ALLOCA/IR_STORE instructions in the entry block */
    for(size_t k = 0; k < n; ++k){
        ir_value_t* p = f->params[k];
        bind_value(cg, p, LLVMGetParam(fn, (unsigned)k));
    }

    /* emit each block */
    for(size_t k = 0; k < block_count; ++k){
        LLVMPositionBuilderAtEnd(cg->builder, llvm_blocks[k]);
        for(ir_instr_t* i = blocks[k]->first; i; i = i->next){
            emit_instr(cg, map, block_count, i);
        }
    }

    free(blocks);
    free(llvm_blocks);
    free(map);
    return true;
}

bool codegen_emit(codegen_t* cg, ir_t* ir)
{
    if(!cg || !ir || !ir->module) return false;

    declare_runtime(cg);

    /* pass 1: declare all functions and globals */
    for(ir_func_t* f = ir->module->first_func; f; f = f->next){
        size_t n = f->param_count;
        LLVMTypeRef* param_types = NULL;
        if(n){
            param_types = malloc(sizeof(LLVMTypeRef) * n);
            if(!param_types) return false;
            for(size_t k = 0; k < n; ++k){
                param_types[k] = llvm_type_of_dt(cg, f->params[k]->type);
            }
        }
        LLVMTypeRef ret_type = llvm_type_of_dt(cg, f->return_type);
        LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types ? param_types : (LLVMTypeRef[]){0},
                                                 n, false);
        LLVMAddFunction(cg->module, f->name.data ? f->name.data : "anon", func_type);
        free(param_types);
    }

    /* declare globals */
    for(size_t k = 0; k < ir->module->global_count; ++k){
        ir_value_t* g = ir->module->globals[k];
        if(!g) continue;
        LLVMTypeRef ty = llvm_type_of_dt(cg, g->type);
        LLVMValueRef global = LLVMAddGlobal(cg->module, ty, g->name.data ? g->name.data : "global");
        if(!global) return false;
        LLVMSetLinkage(global, LLVMExternalLinkage);
        LLVMSetGlobalConstant(global, false);
        /* bind the IR value name to the LLVM global */
        hm_insert(cg->named_values, g->name.data ? g->name.data : "", global);
    }

    /* pass 2: emit bodies */
    for(ir_func_t* f = ir->module->first_func; f; f = f->next){
        if(!emit_function(cg, f)) return false;
    }

    return true;
}

LLVMModuleRef codegen_module(codegen_t* cg)
{
    return cg ? cg->module : NULL;
}

void codegen_dump(codegen_t* cg, FILE* out)
{
    if(!cg || !cg->module) return;
    char* out_str = LLVMPrintModuleToString(cg->module);
    if(out_str){
        fputs(out_str, out);
        LLVMDisposeMessage(out_str);
    }
}

bool codegen_write_ll(codegen_t* cg, const char* path)
{
    if(!cg || !cg->module) return false;
    char* err = NULL;
    int res = LLVMPrintModuleToFile(cg->module, path, &err);
    if(err) LLVMDisposeMessage(err);
    return res == 0;
}

bool codegen_write_bitcode(codegen_t* cg, const char* path)
{
    if(!cg || !cg->module) return false;
    int res = LLVMWriteBitcodeToFile(cg->module, path);
    return res == 0;
}

/* -------------------------------------------------------------------------- */
/* lifecycle                                                                  */
/* -------------------------------------------------------------------------- */

codegen_t* codegen_new(void)
{
    codegen_t* cg = malloc(sizeof(*cg));
    if(!cg) return NULL;
    memset(cg, 0, sizeof(*cg));
    cg->ctx = LLVMContextCreate();
    cg->module = LLVMModuleCreateWithNameInContext(CODEGEN_MODULE_NAME, cg->ctx);
    cg->builder = LLVMCreateBuilderInContext(cg->ctx);
    cg->named_values = NULL;
    return cg;
}

void codegen_free(codegen_t* cg)
{
    if(!cg) return;
    if(cg->named_values) free_hashmap(cg->named_values);
    if(cg->builder) LLVMDisposeBuilder(cg->builder);
    if(cg->module) LLVMDisposeModule(cg->module);
    if(cg->ctx) LLVMContextDispose(cg->ctx);
    free(cg);
}
