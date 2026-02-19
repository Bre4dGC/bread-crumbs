#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static struct timeval start, end;
static double times[10];
static int time_count = 0;

void bm_start(void)
{
    gettimeofday(&start, NULL);
}

void bm_stop(void)
{
    gettimeofday(&end, NULL);
    if(time_count < 10){
        times[time_count++] = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    }
}

void bm_print(const char *test_name)
{
    double total = 0.0, min = 1e9, max = 0.0;
    for(int i = 0; i < time_count; i++){
        total += times[i];
        if(times[i] < min) min = times[i];
        if(times[i] > max) max = times[i];
    }
    double avg = total / time_count;

    printf("\n\033[1m[Benchmark]\033[0m %s\n", test_name);
    printf("\033[1mRuns: \033[0;90m%d\033[0m\n", time_count);
    printf("\033[1mTotal Time: \033[0;90m%.6f seconds\033[0m\n", total);
    printf("\033[1mAverage Time: \033[0;90m%.6f seconds\033[0m\n", avg);
    printf("\033[1mMin Time: \033[0;90m%.6f seconds\033[0m\n", min);
    printf("\033[1mMax Time: \033[0;90m%.6f seconds\033[0m\n", max);
}

void bm_reset(void)
{
    time_count = 0;
}
