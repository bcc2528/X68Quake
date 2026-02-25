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
// vid_x68.c -- X68000 video driver to aid porting efforts

#pragma GCC push_options
#pragma GCC optimize ("O2")

#include "quakedef.h"
#include "d_local.h"

#include "x68quake.h"
#include "vid_x68k.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>

viddef_t	vid;				// global video state

static int ssp;
static int crt_mode;
static int page;

#define	BASEWIDTH	320
#define	BASEHEIGHT	200

static byte	*vid_surfcache;
static size_t	vid_surfcache_size;
static int	vid_highhunkmark;

void	VID_SetPalette (unsigned char *palette)
{
	unsigned short r, g, b;

	register int i = 0;
	do
	{
		r = (*palette++) & 0xf8;
		g = (*palette++) & 0xf8;
		b = (*palette++) & 0xf8;
		gpal[i] = ((g << 8) | (r << 3) | (b >> 2)) + 1;
	} while(++i < 256);
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

/*
================
VID_AllocBuffers
================
*/
qboolean VID_AllocBuffers (int width, int height)
{
	int		tsize, tbuffersize;

	//tbuffersize = framebuffer + z-buffer
	tbuffersize = (width * height) +
			(width * height * sizeof (*d_pzbuffer));

	tsize = D_SurfaceCacheForRes (width, height);

	tbuffersize += tsize;

	vid_surfcache_size = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Hunk_FreeToHighMark (vid_highhunkmark);
		d_pzbuffer = NULL;
	}

	vid_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = Hunk_HighAllocName (tbuffersize, "video");

	vid_surfcache = (byte *)d_pzbuffer
		+ width * height * sizeof (*d_pzbuffer);
	
	vid.buffer = (void *)( (byte *)vid_surfcache + vid_surfcache_size);
	vid.conbuffer = vid.buffer;
	vid.direct = vid.conbuffer;

	Con_Printf ("Resolution: %d x %d\n",width, height);

	return true;
}

void	VID_Init (unsigned char *palette)
{
	ssp = B_SUPER(0);

	crt_mode = CRTMOD(-1);

	// 320 * 208 24(25.42)KHz
	#define VRAMOFFSETX 0
	#define VRAMOFFSETY 4
	*crtc_r00 = 0x38;
	*crtc_r01 = 0x07;
	*crtc_r02 = 0x0a;
	*crtc_r03 = 0x32;
	*crtc_r04 = 0x1d0;
	*crtc_r05 = 0x07;
	*crtc_r06 = 0x20;
	*crtc_r07 = 0x1c0;
	*crtc_r08 = 0x1b;
	*crtc_r20 = 0x1110; // 256x256 256 color, 31khz, gvram enabled

	/* gvram settings
	// 0 = 512x512, 16 color 
	// 4 = 1024x1024, 16
	// 1 = 512x512, 256
	// 2 = dont use
	// 3 = 512x512, 65536(swizzled)*/
	*vidcon_r0 = 0x1;	

	/* set scrolls to 0*/
	*crtc_r10 = 0;
	*crtc_r11 = 0;
	*crtc_r12 = 0;
	*crtc_r13 = 0;
	*crtc_r14 = 0;
	*crtc_r15 = 0;
	*crtc_r16 = 0;
	*crtc_r17 = 0;
	*crtc_r18 = 0;
	*crtc_r19 = 0;

	/* text, graphics, bg/sprite, 
	// graphics layers priority, 0, 1, 2, 3*/
	*vidcon_r1 = 0x24E4;
	/* set Hidden GVRAM layers*/
	*vidcon_r2 = 0;
	
	/* 8x8 31khz BGn*/
	/**cynthia_res = 0x10;*/
	
	C_CUROFF();
	
	memset(gvram[0], 0x10, 0x100000);	/* clear all gvram*/
	/* set GVRAM1 layers visible*/
	*vidcon_r2 = 12;

	page = 0;
	gvram[0] += (VRAMOFFSETY * GVRAMWIDTH) + VRAMOFFSETX;
	gvram[1] += (VRAMOFFSETY * GVRAMWIDTH) + VRAMOFFSETX;


	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;

	VID_AllocBuffers(BASEWIDTH, BASEHEIGHT);
	D_InitCaches (vid_surfcache, vid_surfcache_size);

	X68K_Init();
}

void	VID_Shutdown (void)
{
	X68K_Quit();

	CRTMOD(crt_mode);
	C_CURON();

        __asm__ volatile ("subq.l #8,sp\n"
                          "move.l sp,usp\n"
                          "addq.l #4,sp\n"
                          "move.l %0,(sp)\n"
                          "jsr (%1)\n"
                          "addq.l #4,sp" :: "d"(ssp), "a"(B_SUPER));
}

void	VID_Update (vrect_t *rects)
{
	int x, y;
	short *dest;
	char *src;

	dest = gvram[page];
	src = (char *)vid.buffer;

	for(y = 0;y < BASEHEIGHT;y++)
	{
		for(x = 0;x < BASEWIDTH;x++)
		{
			*dest++ = *src++;
		}
		dest += GVRAMWIDTH - BASEWIDTH;
	}

	page = 1 - page;
	*vidcon_r2 = 12 - (page * 9);
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


#pragma GCC pop_options
