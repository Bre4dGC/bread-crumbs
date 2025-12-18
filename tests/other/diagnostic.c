#include <stdio.h>

#include "compiler/core/diagnostic.h"
#include "common/benchmark.h"

int main(void)
{    
    bench_start();

    report_t* err = new_report(
        SEVERITY_ERROR, ERROR_UNCLOSED_STRING, (location_t){10, 23}, 1, "var name: str = \"bread"
    );
    
    if(err){
        print_report(err);
        free_report(err);
    }
    else {
        printf("Failed to create report\n");
    }

    bench_stop();
    bench_print("Test diagnostic");

    return 0;
}
