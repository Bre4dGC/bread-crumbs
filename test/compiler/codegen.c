#include "compiler/backend/codegen.h"
#include "common/benchmark.h"

int main(void)
{
    bench_start();

    bench_stop();
    bench_print("Test codegen");
    return 0;
}