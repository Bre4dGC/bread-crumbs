#include "core/common/source.h"
#include "compiler/backend/codegen.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    bm_stop();
    bm_print("Test codegen");
    return 0;
}