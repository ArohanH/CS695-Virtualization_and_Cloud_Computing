#include <stddef.h>
#include <stdint.h>

inline uint64_t rdtsc() {
    uint32_t low, high;
    asm volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{
	
	/* Write code here */

	int prod_p=0;
	int cons_p=0;
	int producer_buffer[20];
	for (int i=0; i < 20; i++) {
		producer_buffer[i] = -1;
	}
	asm ("out %0,%1" : /* empty */ : "a"((uint32_t)&(producer_buffer[0])), "Nd"(0xE9) : "memory");
	for (;;){
		int RandomNumber1 = rdtsc()%11;
		asm volatile("in %1, %0" : "=a"(prod_p) : "Nd"(0xEB) : "memory");
		asm volatile("in %1, %0" : "=a"(cons_p) : "Nd"(0xEC) : "memory");
		for(int i = 0; i < RandomNumber1 && producer_buffer[prod_p] == -1; i++){
			int RandomNumber2 = rdtsc()%11;
			producer_buffer[prod_p] = RandomNumber2;
			prod_p++;
			prod_p = prod_p%20;
		}
		cons_p=cons_p;
		asm("out %0, %1" : /* empty */ : "a"(prod_p), "Nd"(0xE3) : "memory");
	}
	*(long *)0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a"(42) : "memory");
}
