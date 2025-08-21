#include <stddef.h>
#include <stdint.h>

static void outb(uint16_t port, uint8_t value)
{
	asm("outb %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

void HC_print8bit(uint8_t val)
{
	outb(0xE9, val);
}

void HC_print32bit(uint32_t val)
{
	//val++;
	/* Write code here */
	asm("outl %0,%1" : /* empty */ : "a"(val), "Nd"(0xEA) : "memory");
}

uint32_t HC_numExits()
{
	uint32_t val = 0;
	/* Write code here */
	asm volatile ("inl %1, %0" : "=a"(val) : "Nd"(0xEB) : "memory");
	return val;
}

void HC_printStr(char *str)
{
	//str++;
	/* Write code here */
	asm("outl %0, %1" : : "a" ((uint32_t)(long)str), "Nd" (0xEC) : "memory");
}

char *HC_numExitsByType()
{
	/* Write code here */
	uint32_t ret32 = 0;
	asm volatile ("inl %1, %0" : "=a"(ret32) : "Nd"(0xED) : "memory");
	return (char *)(uintptr_t)ret32;
}

uint32_t HC_gvaToHva(uint32_t gva)
{
	//gva++;
	// uint32_t hva = 0;
	/* Write code here */
	uintptr_t addr = (uintptr_t)&gva;
    // If needed, cast addr to uint32_t (assuming the address fits in 32 bits).
    uint32_t addr32 = (uint32_t)addr;
    asm ("outl %0, %1" : : "a" (addr32), "Nd" (0xEE) : "memory");
    return gva;
}

void
	__attribute__((noreturn))
	__attribute__((section(".start")))
	_start(void)
{
	const char *p;

	for (p = "Hello 695!\n"; *p; ++p)
		HC_print8bit(*p);

	/*----------Don't modify this section. We will use grading script---------*/
	/*---Your submission will fail the testcases if you modify this section---*/
	HC_print32bit(2048);
	HC_print32bit(4294967295);

	uint32_t num_exits_a, num_exits_b;
	num_exits_a = HC_numExits();

	char *str = "CS695 Assignment 2\n";
	HC_printStr(str);

	num_exits_b = HC_numExits();

	HC_print32bit(num_exits_a);
	HC_print32bit(num_exits_b);

	char *firststr = HC_numExitsByType();
	uint32_t hva;
	hva = HC_gvaToHva(1024);
	HC_print32bit(hva);
	hva = HC_gvaToHva(4294967295);
	HC_print32bit(hva);
	char *secondstr = HC_numExitsByType();

	HC_printStr(firststr);
	HC_printStr(secondstr);
	/*------------------------------------------------------------------------*/

	*(long *)0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a"(42) : "memory");
}
