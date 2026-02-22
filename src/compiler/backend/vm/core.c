#include "compiler/backend/vm/core.h"

vm_t* new_vm(arena_t* arena)
{
    // TODO: implement
}

void vm_execute(vm_t* vm)
{
    vm->stack->pc = 0;
    vm->stack->sp = 0;
    vm->running = true;

    while (vm->running && vm->stack->pc < vm->count){
        ir_instr_t instruction = vm->code[vm->stack->pc];

        switch (instruction.op){
            case OP_PUSH:
                vm->stack->s[vm->stack->sp++] = instruction.data.int_value;
                break;

            case OP_ADD: {
                int64_t b = vm->stack->s[--vm->stack->sp];
                int64_t a = vm->stack->s[--vm->stack->sp];
                vm->stack->s[vm->stack->sp++] = a + b;
                break;
            }

            case OP_SUB: {
                int64_t b = vm->stack->s[--vm->stack->sp];
                int64_t a = vm->stack->s[--vm->stack->sp];
                vm->stack->s[vm->stack->sp++] = a - b;
                break;
            }

            case OP_JUMP:
                vm->stack->pc = instruction.data.int_value;
                continue;

            case OP_JUMP_IFNOT: {
                int64_t cond = vm->stack->s[--vm->stack->sp];
                if(cond == 0){
                    vm->stack->pc = instruction.data.int_value;
                    continue;
                }
                break;
            }

            case OP_RETURN:
                vm->running = false;
                break;

            default:
                vm->running = false;
                break;
        }
        vm->stack->pc++;
    }
}

void free_vm(vm_t* vm)
{
    // TODO: implement
}
