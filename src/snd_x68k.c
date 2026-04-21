/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_x68k.c -- PCM8A output sound

#pragma GCC push_options
#pragma GCC optimize ("O1")

#include "quakedef.h"
#include "pcm8a.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>

#define ADPCM_RATE      15625
#define ADPCM_MODE      0x0403
//#define ADPCM_RATE      10416
//#define ADPCM_MODE      0x0303

cvar_t bgmvolume = {"bgmvolume", "1", true};
cvar_t volume = {"volume", "1.0", true};

cvar_t nosound = {"nosound", "0"};
cvar_t precache = {"precache", "1"};
cvar_t ambient_level = {"ambient_level", "0.3"};
cvar_t ambient_fade = {"ambient_fade", "100"};

// =======================================================================
// Internal sound data & structures
// =======================================================================

vec3_t		listener_origin;
vec3_t		listener_forward;
vec3_t		listener_right;
vec3_t		listener_up;
vec_t		sound_nominal_clip_dist=1000.0;

int sound_started=0;
unsigned char *adpcm_data;
int PCM8A_ch;

#define	MAX_SFX		512
sfx_t		*known_sfx;
int			num_sfx = 0;

static qboolean	snd_ambient = 0;
sfx_t		*ambient_sfx[NUM_AMBIENTS];


/*
================
ResampleSfx
================
*/
void ResampleSfx (sfx_t *sfx, int inrate, int inwidth, byte *data)
{
	int		outcount;
	int		srcsample;
	float	stepscale;
	int		i;
	int		sample, samplefrac, fracstep;
	sfxcache_t	*sc;
	
	sc = Cache_Check (&sfx->cache);
	if (!sc)
		return;

	stepscale = (float)inrate / ADPCM_RATE;	// this is usually 0.5, 1, or 2

	outcount = sc->length / stepscale;
	sc->length = outcount;
	if (sc->loopstart != -1)
		sc->loopstart = sc->loopstart / stepscale;

	sc->speed = ADPCM_RATE;
	sc->width = 1;
	sc->stereo = 0;

// resample / decimate to the current source rate

	samplefrac = 0;
	fracstep = stepscale*256;
	for (i=0 ; i<outcount ; i++)
	{
		srcsample = samplefrac >> 8;
		samplefrac += fracstep;

		if (inwidth == 2)
			sample = LittleShort ( ((short *)data)[srcsample] );
		else
			sample = (int)( (unsigned char)(data[srcsample]) - 128) << 8;

		((signed char *)sc->data)[i] = sample >> 8;
	}
}

/*
==============
S_LoadSound
==============
*/
sfxcache_t *S_LoadSound (sfx_t *s)
{
	char	namebuffer[256];
	byte	*data;
	wavinfo_t	info;
	int		len;
	float	stepscale;
	sfxcache_t	*sc;
	byte	stackbuf[1*1024];		// avoid dirtying the cache heap

// see if still in memory
	sc = Cache_Check (&s->cache);
	if (sc)
		return sc;

//Con_Printf ("S_LoadSound: %x\n", (int)stackbuf);
// load it in
	Q_strcpy(namebuffer, "sound/");
	Q_strcat(namebuffer, s->name);

//	Con_Printf ("loading %s\n",namebuffer);

	data = COM_LoadStackFile(namebuffer, stackbuf, sizeof(stackbuf));

	if (!data)
	{
		Con_Printf ("Couldn't load %s\n", namebuffer);
		return NULL;
	}

	info = GetWavinfo (s->name, data, com_filesize);
	if (info.channels != 1)
	{
		Con_Printf ("%s is a stereo sample\n",s->name);
		return NULL;
	}

	stepscale = (float)info.rate / ADPCM_RATE;	
	len = info.samples / stepscale;

	len = len * info.width * info.channels;

	sc = Cache_Alloc ( &s->cache, len + sizeof(sfxcache_t), s->name);
	if (!sc)
		return NULL;
	
	sc->length = info.samples;
	sc->loopstart = info.loopstart;
	sc->speed = info.rate;
	sc->width = info.width;
	sc->stereo = info.channels;

	ResampleSfx(s, sc->speed, sc->width, data + info.dataofs);

	return sc;
}



/*
===============================================================================

WAV loading

===============================================================================
*/


byte	*data_p;
byte 	*iff_end;
byte 	*last_chunk;
byte 	*iff_data;
int 	iff_chunk_len;


short GetLittleShort(void)
{
	short val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	data_p += 2;
	return val;
}

int GetLittleLong(void)
{
	int val = 0;
	val = *data_p;
	val = val + (*(data_p+1)<<8);
	val = val + (*(data_p+2)<<16);
	val = val + (*(data_p+3)<<24);
	data_p += 4;
	return val;
}

void FindNextChunk(char *name)
{
	while (1)
	{
		data_p=last_chunk;

		if (data_p >= iff_end)
		{	// didn't find the chunk
			data_p = NULL;
			return;
		}
		
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if (iff_chunk_len < 0)
		{
			data_p = NULL;
			return;
		}
//		if (iff_chunk_len > 1024*1024)
//			Sys_Error ("FindNextChunk: %i length is past the 1 meg sanity limit", iff_chunk_len);
		data_p -= 8;
		last_chunk = data_p + 8 + ( (iff_chunk_len + 1) & ~1 );
		if (!Q_strncmp((char *)data_p, name, 4))
			return;
	}
}

void FindChunk(char *name)
{
	last_chunk = iff_data;
	FindNextChunk (name);
}


void DumpChunks(void)
{
	char	str[5];
	
	str[4] = 0;
	data_p=iff_data;
	do
	{
		memcpy (str, data_p, 4);
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		Con_Printf ("0x%x : %s (%d)\n", (int)(data_p - 4), str, iff_chunk_len);
		data_p += (iff_chunk_len + 1) & ~1;
	} while (data_p < iff_end);
}

/*
============
GetWavinfo
============
*/
wavinfo_t GetWavinfo (char *name, byte *wav, int wavlength)
{
	wavinfo_t	info;
	int     i;
	int     format;
	int		samples;

	memset (&info, 0, sizeof(info));

	if (!wav)
		return info;
		
	iff_data = wav;
	iff_end = wav + wavlength;

// find "RIFF" chunk
	FindChunk("RIFF");
	if (!(data_p && !Q_strncmp((char *)data_p+8, "WAVE", 4)))
	{
		Con_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}

// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk("fmt ");
	if (!data_p)
	{
		Con_Printf("Missing fmt chunk\n");
		return info;
	}
	data_p += 8;
	format = GetLittleShort();
	if (format != 1)
	{
		Con_Printf("Microsoft PCM format only\n");
		return info;
	}

	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4+2;
	info.width = GetLittleShort() / 8;

// get cue chunk
	FindChunk("cue ");
	if (data_p)
	{
		data_p += 32;
		info.loopstart = GetLittleLong();
//		Con_Printf("loopstart=%d\n", sfx->loopstart);

	// if the next chunk is a LIST chunk, look for a cue length marker
		FindNextChunk ("LIST");
		if (data_p)
		{
			if (!strncmp ((char *)data_p + 28, "mark", 4))
			{	// this is not a proper parse, but it works with cooledit...
				data_p += 24;
				i = GetLittleLong ();	// samples in loop
				info.samples = info.loopstart + i;
//				Con_Printf("looped length: %i\n", i);
			}
		}
	}
	else
		info.loopstart = -1;

// find data chunk
	FindChunk("data");
	if (!data_p)
	{
		Con_Printf("Missing data chunk\n");
		return info;
	}

	data_p += 4;
	samples = GetLittleLong () / info.width;

	if (info.samples)
	{
		if (samples < info.samples)
			Sys_Error ("Sound %s has a bad loop length", name);
	}
	else
		info.samples = samples;

	info.dataofs = data_p - wav;
	
	return info;
}


void S_Init (void)
{
	Cvar_RegisterVariable(&nosound);
	Cvar_RegisterVariable(&volume);
	Cvar_RegisterVariable(&precache);
	Cvar_RegisterVariable(&bgmvolume);
	Cvar_RegisterVariable(&ambient_level);
	Cvar_RegisterVariable(&ambient_fade);

	if (COM_CheckParm("-nosound"))
	{
		Con_Printf("\nNo Sound\n");
		return;
	}
	if(pcm8a_check() < 0)
	{
		Con_Printf("\nPCM8A Not detected\n");
		return;
	}
	Con_Printf("\nSound Initialization\n");

	sound_started = 1;
	PCM8A_ch = 0;

	known_sfx = Hunk_AllocName (MAX_SFX*sizeof(sfx_t), "sfx_t");
	num_sfx = 0;

	ambient_sfx[AMBIENT_WATER] = S_PrecacheSound ("ambience/water1.wav");
	ambient_sfx[AMBIENT_SKY] = S_PrecacheSound ("ambience/wind2.wav");
	ambient_sfx[2] = 0;
	ambient_sfx[3] = 0;
}

void S_AmbientOff (void)
{
	snd_ambient = false;
}

void S_AmbientOn (void)
{
	snd_ambient = true;
}

void S_Shutdown (void)
{
	if (!sound_started)
		return;

	pcm8a_pause();
	pcm8a_stop();
}

sfx_t *S_FindName (char *name)
{
	int		i;
	sfx_t	*sfx;

	if (!name)
		Sys_Error ("S_FindName: NULL\n");

	if (Q_strlen(name) >= MAX_QPATH)
		Sys_Error ("Sound name too long: %s", name);

// see if already loaded
	for (i=0 ; i < num_sfx ; i++)
		if (!Q_strcmp(known_sfx[i].name, name))
		{
			return &known_sfx[i];
		}

	if (num_sfx == MAX_SFX)
		Sys_Error ("S_FindName: out of sfx_t");
	
	sfx = &known_sfx[i];
	strcpy (sfx->name, name);

	num_sfx++;
	
	return sfx;
}

void S_TouchSound (char *sample)
{
	sfx_t	*sfx;

	if (!sound_started)
		return;

	sfx = S_FindName (sample);
	Cache_Check (&sfx->cache);
}

void S_ClearBuffer (void)
{
}

void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation)
{
	channel_t *target_chan, *check;
	channel_t channel;
	sfxcache_t	*sc;
	int lr_output;
	int vol;

	if (!sound_started)
		return;

	if (!sfx)
		return;

	if (nosound.value)
		return;

	sc = S_LoadSound (sfx);
	if (!sc)
	{
		return;		// couldn't load the sound's data
	}

	target_chan = &channel;

// spatialize
	vol = fvol * 255;
	memset (target_chan, 0, sizeof(*target_chan));
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);

	if(!target_chan->leftvol)
	{
		return;
	}
	vol = target_chan->leftvol;
	vol >>= 4;
	switch(vol & 0xf)
	{
		case 15:
		case 14:
		case 13:
		case 12:
			vol = 0x80603;
			break;
		case 11:
		case 10:
			vol = 0x70603;
			break;
		case 9:
		case 8:
			vol = 0x60603;
			break;
		case 7:
		case 6:
			vol = 0x50603;
			break;
		case 5:
			vol = 0x40603;
			break;
		case 4:
			vol = 0x30603;
			break;
		case 3:
			vol = 0x20603;
			break;
		case 2:
			vol = 0x10603;
			break;
		case 1:
		case 0:
			vol = 0x603;
			break;
	}
	pcm8a_play(PCM8A_ch, vol, sc->length, sc->data);
	PCM8A_ch++;
	PCM8A_ch&=7;
}

void S_StopSound (int entnum, int entchannel)
{
	if (!sound_started)
		return;

	pcm8a_pause();
	pcm8a_stop();
}

sfx_t *S_PrecacheSound (char *sample)
{
	sfx_t	*sfx;

	if (!sound_started || nosound.value)
		return NULL;

	sfx = S_FindName (sample);
	
// cache it in
	if (precache.value)
		S_LoadSound (sfx);
	
	return sfx;
}

void S_ClearPrecache (void)
{
}

/*
===================
S_UpdateAmbientSounds
===================
*/
void S_UpdateAmbientSounds (void)
{
	mleaf_t		*l;
	float		vol;
	int			ambient_channel, pcm_mode;
	sfx_t	*sfx;
	sfxcache_t	*sc;

	if (!snd_ambient)
		return;

// calc ambient sound levels
	if (!cl.worldmodel)
		return;

	l = Mod_PointInLeaf (listener_origin, cl.worldmodel);
	if (!l || !ambient_level.value)
	{
		return;
	}

	for (ambient_channel = 0 ; ambient_channel< NUM_AMBIENTS ; ambient_channel++)
	{
		sfx = ambient_sfx[ambient_channel];

		if(!sfx)
		{
			continue;
		}

		sc = S_LoadSound (sfx);
		if (!sc)
		{
			continue;
		}

		vol = ambient_level.value * l->ambient_sound_level[ambient_channel];
		if (vol < 8)
			vol = 0;
	}
}

void S_Update (vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{
	if (!sound_started)
		return;

	VectorCopy(origin, listener_origin);
	VectorCopy(v_forward, listener_forward);
	VectorCopy(v_right, listener_right);
	VectorCopy(v_up, listener_up);

	S_UpdateAmbientSounds();
}

void S_StopAllSounds (qboolean clear)
{
	if (!sound_started)
		return;

	if (clear)
	{
		pcm8a_pause();
		pcm8a_stop();
	}
}

void S_BeginPrecaching (void)
{
}

void S_EndPrecaching (void)
{
}

void S_ExtraUpdate (void)
{
}

void S_LocalSound (char *s)
{
	sfx_t	*sfx;

	if (nosound.value)
		return;
	if (!sound_started)
		return;
		
	sfx = S_PrecacheSound (s);
	if (!sfx)
	{
		Con_Printf ("S_LocalSound: can't cache %s\n", s);
		return;
	}
	S_StartSound (cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}

void SND_Spatialize(channel_t *ch)
{
	vec_t dot;
	vec_t ldist, rdist, dist;
	vec_t lscale, rscale, scale;
	vec3_t source_vec;
	sfx_t *snd;

// anything coming from the view entity will allways be full volume
	if (ch->entnum == cl.viewentity)
	{
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}

// calculate stereo seperation and distance attenuation

	snd = ch->sfx;
	VectorSubtract(ch->origin, listener_origin, source_vec);
	
	dist = VectorNormalize(source_vec) * ch->dist_mult;
	
	dot = DotProduct(listener_right, source_vec);

	//rscale = 1.0 + dot;
	//lscale = 1.0 - dot;
	// ADPCM of X68000 is monaural
	rscale = 1.0;
	lscale = 1.0;

// add in distance effect
	scale = (1.0 - dist) * rscale;
	ch->rightvol = (int) (ch->master_vol * scale);
	if (ch->rightvol < 0)
		ch->rightvol = 0;

	scale = (1.0 - dist) * lscale;
	ch->leftvol = (int) (ch->master_vol * scale);
	if (ch->leftvol < 0)
		ch->leftvol = 0;
}           

#pragma GCC pop_options