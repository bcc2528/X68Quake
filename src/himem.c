#include <stddef.h>

#pragma GCC push_options
#pragma GCC optimize ("O2")

void *himem_malloc(size_t size)
{
	register void *ret __asm__("a1");
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x00f8, d0\n"
		"moveq   #1, d1\n"
		"move.l  %0, d2\n"
		"trap    #15\n" :: "d"(size));

	return (r_d0 == 0) ? ret : NULL;
}

int himem_free(void* ptr)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x00f8, d0\n"
		"moveq   #2, d1\n"
		"move.l  %0, d2\n"
		"trap    #15\n" :: "a"(ptr));

	return r_d0;
}

size_t himem_getsize(void)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"moveq  #0x00f8, d0\n"
		"moveq  #3, d1\n"
		"trap   #15\n");

	return r_d0;
}

int himem_resize(void* ptr, size_t size)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x00f8, d0\n"
		"moveq   #4, d1\n"
		"move.l  %1, d2\n"
		"move.l  %2, d3\n"
		"trap    #15\n"
		: "=d"(r_d0)
		: "r"(ptr), "r"(size)
		: "d1", "d2", "d3"
		);

	return r_d0;
}

#pragma GCC pop_options
