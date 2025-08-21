#include <stddef.h>
#include <stdint.h>

#define NUM_ITEMS 5
int producer_array[NUM_ITEMS];

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{

	/* write code here */


	asm("outl %0, %1" : : "a" ((uint32_t)(uintptr_t)producer_array), "Nd" (0xE9) : "memory");
	int k = 0;
	for (;;) {
		for(int i=0;i<NUM_ITEMS;i++){
			producer_array[i] = k;
			k++;
		}
		asm("outl %0, %1" : : "a" ((uint32_t)(uintptr_t)producer_array), "Nd" (0xEC) : "memory");
	}
}
