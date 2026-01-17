#include <stdio.h>

#include "compiler/core/diagnostic.h"
#include "common/benchmark.h"

int main(void)
{    
    bench_start();

    bench_stop();
    bench_print("Test diagnostic");

    return 0;
}
