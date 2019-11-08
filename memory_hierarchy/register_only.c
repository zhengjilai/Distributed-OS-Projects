#include <stdio.h> 
#include <time.h>
#define MAXNUM 100000000
/**
 * This C program place variables i and sum in register only
 * Such placement is enforced by writing assemble code in C
 */

int main() { 
    int maxnum = MAXNUM;
    int res = -1;
    int i = 0;
    clock_t start, end;
    start = clock();
    // the assemble language code in C
    __asm__ __volatile__(
        "_start: pushq %%rax \n\t"
        "pushq %%rbx \n\t"
        "pushq %%rcx \n\t"
        "xor %%eax, %%eax \n\t"
        "mov %1, %%ebx \n\t"
        "xor %%ecx, %%ecx\n\t"
        "_loop: add $4, %%ecx \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "cmpl %%ecx, %%ebx \n\t"
        "jg _loop \n\t"
        "mov %%eax, %0 \n\t"
        "popq %%rcx \n\t"
        "popq %%rbx \n\t"
        "popq %%rax \n\t"
        : "=r" (res)
        : "r" (maxnum), "r"(i) 
        : "cc","%rax","%rbx","%rcx"
    );
    end = clock();
    printf("Time for register only is: %.8f seconds\n", (double)(end - start)/CLOCKS_PER_SEC);
    printf("Result for register only accumulation is %d\n", res);
} 
