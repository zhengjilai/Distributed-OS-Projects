#include <stdio.h> 
#define MAXNUM 100000000
/**
 * This C program place variables i and sum in register only
 * Such placement is enforced by writing assemble code in C
 */

int main() { 
    int sum = 0;
    int maxnum = MAXNUM;
    // the assemble language code in C
    __asm__ __volatile__(
        "_start: \n"
        "pushq %%rax \n\t"
        "pushq %%rbx \n\t"
        "pushq %%rcx \n\t"
        "movl %2, %%eax \n\t"
        "movl %1, %%ebx \n\t"
        "xor %%ecx, %%ecx\n\t"
        "_loop: \n"
        "add 4, %%ecx \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "inc %%eax \n\t"
        "cmpl %%eax, %%ebx \n\t"
        "jl _loop \n\t"
        "movl %%eax, %0 \n\t"
        "popq %%rcx \n\t"
        "popq %%rbx \n\t"
        "popq %%rax \n\t"
        : "=r" (sum)
        : "r" (maxnum), "r" (sum)
    );
    printf("Result for register only accumulation is %d\n", sum);
} 
