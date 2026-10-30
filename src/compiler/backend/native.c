#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "compiler/backend/native.h"

struct native {
    int dummy;
};

native_t* native_create(void)
{
    native_t* nat = calloc(1, sizeof(*nat));
    return nat;
}

void native_free(native_t* nat)
{
    if(nat) free(nat);
}

bool native_emit_object(native_t* nat, codegen_t* cg, const char* output_path)
{
    if(!nat || !cg || !output_path) return false;

    /* Write LLVM IR to a temporary .ll file */
    const char* ll_path = "/tmp/brc_temp.ll";
    if(!codegen_write_ll(cg, ll_path)) return false;

    /* Use llc to compile .ll to .o */
    char cmd[1024];
    int n = snprintf(cmd, sizeof(cmd), "llc %s -filetype=obj -o %s", ll_path, output_path);
    if(n < 0 || (size_t)n >= sizeof(cmd)) return false;

    int ret = system(cmd);
    return ret == 0;
}
