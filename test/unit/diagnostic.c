#include "core/diagnostic.h"
#include "core/common/source.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    bm_stop();
    bm_print("Test diagnostic");

    return 0;
}
