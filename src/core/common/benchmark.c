#pragma once
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static struct timeval start, end;
static double times[10];
static int time_count = 0;

void bench_start(void)
{
    gettimeofday(&start, NULL);
}

void bench_stop(void)
{
    gettimeofday(&end, NULL);
    if(time_count < 10){
        times[time_count++] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }
}

void bench_print(const char *test_name)
{
    double total = 0.0, min = 1e9, max = 0.0;
    for(int i = 0; i < time_count; i++){
        total += times[i];
        if(times[i] < min) min = times[i];
        if(times[i] > max) max = times[i];
    }
    double avg = total / time_count;

    printf("\033[1;34m[Benchmark]\033[0m %s\n", test_name);
    printf("  Runs: %d\n", time_count);
    printf("  Total Time: \033[1;32m%.6f seconds\033[0m\n", total);
    printf("  Average Time: \033[1;33m%.6f seconds\033[0m\n", avg);
    printf("  Min Time: \033[1;36m%.6f seconds\033[0m\n", min);
    printf("  Max Time: \033[1;31m%.6f seconds\033[0m\n", max);
}

void bench_reset(void)
{
    time_count = 0;
}
