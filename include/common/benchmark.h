#include <stdio.h>
#include <sys/time.h>

enum bench_time {
    BENCH_TIME_S,
    BENCH_TIME_MS,
    BENCH_TIME_MCS,
};

static struct timeval start, end;

void bench_start(void)
{
    gettimeofday(&start, NULL);
}

double bench_gettime(enum bench_time type)
{
    double seconds = (double)(end.tv_sec - start.tv_sec);
    double microseconds = (double)(end.tv_usec - start.tv_usec);
    double total_time = (seconds * 1000000.0) + microseconds;

    return type == BENCH_TIME_S  ? total_time / 1000000.0 :
           type == BENCH_TIME_MS ? total_time / 1000.0    :
                                   total_time;
}

void bench_stop(void)
{
    gettimeofday(&end, NULL);
}

void bench_print(void)
{
    printf("\nBenchmark results:\n");
    printf(" - Seconds: %.5f\n", bench_gettime(BENCH_TIME_S));
    printf(" - Miliseconds: %.5f\n", bench_gettime(BENCH_TIME_MS));
    printf(" - Microseconds: %.5f\n", bench_gettime(BENCH_TIME_MCS));
}