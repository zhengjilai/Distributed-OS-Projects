#include <stdio.h>
#include <time.h>
#define MAXNUM 100000000

/**
 * This C program place variables i and sum in cache only
 * Such placement is natural, since the variables will be in cache by default
 */
int main(){
    int i, sum;
    clock_t start, end;
    start = clock();
    sum = 0;
    for (i = 0; i < MAXNUM; i += 4){
        sum++;
        sum++;
        sum++;
        sum++;
    }
    end = clock();
    printf("Time for cache only is: %.8f seconds\n", (double)(end - start)/CLOCKS_PER_SEC);
    printf("Result for cache only accumulation is %d\n", sum);
}