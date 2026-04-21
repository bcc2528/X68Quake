#pragma GCC push_options
#pragma GCC optimize ("O1")

int pcm8a_check()
{
	// IOCS_ADPCMMOD = 0x67
	// 'PCMA' = 0x50434d41

	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x67, d0\n"
		"move.l  #0x50434d41, d1\n"
		"trap    #15\n");

	return r_d0;
}

int pcm8a_play(short channel, unsigned int mode, unsigned int size, void* addr)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"move.l  %1, d1\n"
		"move.l  %2, d2\n"
		"move.l  %3, a1\n"
		"trap    #2\n" :: "d"(channel), "d"(mode), "d"(size), "a"(addr));

	return r_d0;
}

int pcm8a_play_ally_chain(short channel, unsigned int mode, unsigned int num, void* addr)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"add  #0x10, d0\n"
		"move.l  %1, d1\n"
		"move.l  %2, d2\n"
		"move.l  %3, a1\n"
		"trap    #2\n" :: "d"(channel), "d"(mode), "d"(num), "a"(addr));

	return r_d0;
}

int pcm8a_play_link_ally_chain(short channel, unsigned int mode, void* addr)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"add  #0x20, d0\n"
		"move.l  %1, d1\n"
		"move.l  %2, a1\n"
		"trap    #2\n" :: "d"(channel), "d"(mode), "a"(addr));

	return r_d0;
}

int pcm8a_change(short channel, unsigned int mode)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"add  #0x70, d0\n"
		"move.l  %1, d1\n"
		"trap    #2\n" :: "d"(channel), "d"(mode));

	return r_d0;
}

int pcm8a_getplaysize(short channel)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"add  #0x80, d0\n"
		"trap    #2\n" :: "d"(channel));

	return r_d0;
}

int pcm8a_ch_pause(short channel)
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.w  %0, d0\n"
		"and  #0xf, d0\n"
		"add  #0xb0, d0\n"
		"trap    #2\n" :: "d"(channel));

	return r_d0;
}

int pcm8a_stop()
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x0100, d0\n"
		"trap    #2\n");

	return r_d0;
}

int pcm8a_pause()
{
	register unsigned int r_d0 __asm__("d0");

	__asm__ volatile(
		"move.l  #0x0101, d0\n"
		"trap    #2\n");

	return r_d0;
}

#pragma GCC pop_options
