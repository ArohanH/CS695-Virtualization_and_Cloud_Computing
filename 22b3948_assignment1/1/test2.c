#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/signal.h>
#include <stdint.h>

#define LARGE_SIZE (512 * 1024 * 1024) // 512 MB
#define PAGE_SIZE 4096 // 

bool end = false;

void handler(int signal){
    if(signal==SIGINT){
        end=true;
    }
}

int main(){
    void *memory=mmap(NULL,LARGE_SIZE,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(memory == MAP_FAILED){
        perror("mmap");
        return EXIT_FAILURE;
    }

    printf("Allocated Memory of size %d\n",LARGE_SIZE);

    signal(SIGINT,handler);

    uintptr_t base_address = (uintptr_t) memory;
    //long writes = 0;
    for (uintptr_t addr = base_address; addr < base_address + LARGE_SIZE; addr += PAGE_SIZE) {
        *((volatile char *)addr) = 42; 
    }

    //printf("Performed %ld writes with a stride of %ld bytes: \n", writes, stride);

    while(!end){

    }

    if(munmap(memory,LARGE_SIZE)!=0){
        perror("munmap");
        return EXIT_FAILURE;
    }

    printf("Memory deallocated successfully.\n");

    return EXIT_SUCCESS;
}