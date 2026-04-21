int pcm8a_check();
int pcm8a_play(short channel, unsigned int mode, unsigned int size, void* addr);
int pcm8a_play_ally_chain(short channel, unsigned int mode, unsigned int num, void* addr);
int pcm8a_play_link_ally_chain(short channel, unsigned int mode, void* addr);
int pcm8a_change(short channel, unsigned int mode);
int pcm8a_getplaysize(short channel);
int pcm8a_ch_pause(short channel);
int pcm8a_stop();
int pcm8a_pause();

struct ally_cahin_table { 
	void* addr;
	unsigned short length;
};

struct link_ally_cahin_table { 
	void* top_addr;
	unsigned short length;
	void* next_chain_addr;
};