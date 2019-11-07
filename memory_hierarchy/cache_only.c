#include <stdio.h>
#define MAXNUM 100000000

/**
 * This C program place variables i and sum in cache only
 * Such placement is natural, since the variables will be in cache by default
 */
int main(){
    int i, sum;
    sum = 0;
    for (i = 0; i < MAXNUM; i += 4){
        sum++;
        sum++;
        sum++;
        sum++;
    }
    printf("Result for cache only accumulation is %d\n", sum);
}