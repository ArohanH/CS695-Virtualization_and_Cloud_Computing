#include <stddef.h>
#include <stdint.h>

#define NUM_ITEMS 5
int consumer_array[NUM_ITEMS];

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{
	
	/* Write code here */


	asm("outl %0, %1" : : "a" ((uint32_t)(uintptr_t)consumer_array), "Nd" (0xEA) : "memory");

	for (;;){
		asm("outl %0, %1" : : "a" ((uint32_t)(uintptr_t)consumer_array), "Nd" (0xEB) : "memory");
	}
}
