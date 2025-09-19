#include <string.h>
#include <stdio.h>

#include "vm.h"
#include "debug.h"

static inline int64_t read_i64(struct vm *vm)
{
    int64_t v = 0;
    if (vm->ip + sizeof(int64_t) > vm->capacity) return 0;
    memcpy(&v, vm->code + vm->ip, sizeof(v));
    vm->ip += sizeof(v);
    return v;
}

static inline void push_val(struct vm *vm, int64_t v)
{
    if (vm->sp >= vm->ssize) return; // simple overflow guard
    vm->stack[vm->sp++] = v;
}

static inline int64_t pop_val(struct vm *vm)
{
    if (vm->sp == 0) return 0;
    return vm->stack[--vm->sp];
}

void vm_execute(struct vm* vm)
{
    while (vm->ip < vm->capacity) {
        const enum op_code op = (enum op_code)vm->code[vm->ip++];
        switch (op) {
            case OP_PUSH: {
                const int64_t val = read_i64(vm);
                push_val(vm, val);
                break;
            }

            case OP_POP: {
                (void)pop_val(vm);
                break;
            }

            case OP_DUP: {
                if (vm->sp == 0) break;
                const int64_t top = vm->stack[vm->sp - 1];
                push_val(vm, top);
                break;
            }

            case OP_ADD: {
                const int64_t b = pop_val(vm);
                const int64_t a = pop_val(vm);
                push_val(vm, a + b);
                break;
            }

            case OP_SUB: {
                const int64_t b = pop_val(vm);
                const int64_t a = pop_val(vm);
                push_val(vm, a - b);
                break;
            }

            case OP_MUL: {
                const int64_t b = pop_val(vm);
                const int64_t a = pop_val(vm);
                push_val(vm, a * b);
                break;
            }

            case OP_DIV: {
                const int64_t b = pop_val(vm);
                const int64_t a = pop_val(vm);
                push_val(vm, b == 0 ? 0 : a / b);
                break;
            }

            case OP_JUMP: {
                const int64_t target = read_i64(vm);
                if (target >= 0 && (size_t)target < vm->capacity) vm->ip = (size_t)target;
                break;
            }

            case OP_JUMP_IF: {
                const int64_t target = read_i64(vm);
                const int64_t cond = pop_val(vm);
                if (cond){
                    if (target >= 0 && (size_t)target < vm->capacity) vm->ip = (size_t)target;
                }
                break;
            }

            case OP_JUMP_IFNOT: {
                const int64_t target = read_i64(vm);
                const int64_t cond = pop_val(vm);
                if (!cond){
                    if (target >= 0 && (size_t)target < vm->capacity) vm->ip = (size_t)target;
                }
                break;
            }

            case OP_RETURN: {
                return;
            }

            default: {
                fprintf(stderr, "vm: unknown opcode %d at ip %zu\n", op, vm->ip - 1);
                return;
            }
        }
    }
}
