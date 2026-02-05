#include <stdio.h>

#include "core/diagnostic.h"
#include "core/common/source.h"
#include "core/common/benchmark.h"

int main(void)
{
    bench_start();

    bench_stop();
    bench_print("Test diagnostic");

    return 0;
}
