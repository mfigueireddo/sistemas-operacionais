#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *myallocate(int n);
void initmemorypool(void);
void resetmemorypool(void);

void *memorypool;

int main(void)
{
    int *limite_heap = (int*)myallocate(1);

    initmemorypool();

    

    resetmemorypool();

    return 0;
}

void *myallocate(int n)
{
    return sbrk(n);
}

void initmemorypool(void)
{
    memorypool = sbrk(0);
}

void resetmemorypool(void)
{
    brk(memorypool);
}