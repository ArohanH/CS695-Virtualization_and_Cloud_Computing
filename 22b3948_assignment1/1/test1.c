#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/signal.h>
#include <stdint.h>
#include <errno.h>

#define PAGE_SIZE 4096 // Assuming 4KB page size

bool end = false;

void handler(int signal){
    if(signal==SIGINT){
        end=true;
    }
}

int main(int argc, char *argv[]){
    if(argc!=3){
        fprintf(stderr, "Usage: %s <number_of_pages> <stride_in_bytes>\n",argv[0]);
        return EXIT_FAILURE;
    }

    long num_pages = atol(argv[1]);
    long stride = atol(argv[2]);

    if(num_pages <=0 || stride<=0){
        fprintf(stderr, "Error: Number of pages and stride should be postive integers\n");
        return EXIT_FAILURE;
    }

    size_t allocation_size = num_pages * PAGE_SIZE;

    void *memory=mmap(NULL,allocation_size,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(memory == MAP_FAILED){
        perror("mmap");
        return EXIT_FAILURE;
    }

    printf("Allocated %ld pages (%ld bytes).\n",num_pages, allocation_size);

    signal(SIGINT,handler);

    uintptr_t base_address = (uintptr_t) memory;
    long writes = 0;
    for (uintptr_t addr = base_address; addr < base_address + allocation_size; addr += stride) {
        *((volatile char *)addr) = 42; 
        writes++;
    }

    printf("Performed %ld writes with a stride of %ld bytes: \n", writes, stride);

    while(!end){

    }

    if(munmap(memory,allocation_size)!=0){
        perror("munmap");
        return EXIT_FAILURE;
    }

    printf("Memory deallocated successfully.\n");

    return EXIT_SUCCESS;
}