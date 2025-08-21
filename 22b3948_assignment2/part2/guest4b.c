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
	int consumer_buffer[20];
	for (int i=0; i < 20; i++) {
		consumer_buffer[i] = -1;
	}
	asm ("out %0,%1" : /* empty */ : "a"((uint32_t)&(consumer_buffer[0])), "Nd"(0xEA) : "memory");
	for (;;){
		int RandomNumber1 = rdtsc()%11;
		asm volatile("in %1, %0" : "=a"(prod_p) : "Nd"(0xE4) : "memory");
		asm volatile("in %1, %0" : "=a"(cons_p) : "Nd"(0xE5) : "memory");
		for(int i = 0; i < RandomNumber1 && consumer_buffer[cons_p] != -1; i++){
			consumer_buffer[cons_p] = -1;
			cons_p++;
			cons_p = cons_p%20;
		}
		prod_p = prod_p;
		asm("out %0, %1" : /* empty */ : "a"(cons_p), "Nd"(0xE6) : "memory");
	}

	*(long *)0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a"(42) : "memory");
}
