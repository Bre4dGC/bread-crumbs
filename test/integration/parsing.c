#include "core/ds/arena.h"
#include "core/lang/diagnostic.h"
#include "core/lang/filesystem.h"
#include "core/lang/source.h"
#include "compiler/frontend/parser.h"
#include "../utils/benchmark.h"

int main(void)
{
    bm_start();

    bm_stop();
    bm_print("Test parser");
    return 0;
}
