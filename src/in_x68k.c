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
// in_null.c -- for systems without a mouse

#pragma GCC push_options
#pragma GCC optimize ("O1")

#include "quakedef.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iocslib.h>
#include <doslib.h>

#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball

enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn
};

float	mouse_x, mouse_y;
cvar_t	in_joystick = {"joystick","0", true};
cvar_t	joy_advanced = {"joyadvanced", "0"};
cvar_t	joy_advaxisx = {"joyadvaxisx", "0"};
cvar_t	joy_advaxisy = {"joyadvaxisy", "0"};
cvar_t	joy_advaxisz = {"joyadvaxisz", "0"};
cvar_t	joy_advaxisr = {"joyadvaxisr", "0"};
cvar_t	joy_advaxisu = {"joyadvaxisu", "0"};
cvar_t	joy_advaxisv = {"joyadvaxisv", "0"};
cvar_t	joy_forwardthreshold = {"joyforwardthreshold", "0.15"};
cvar_t	joy_sidethreshold = {"joysidethreshold", "0.15"};
cvar_t	joy_pitchthreshold = {"joypitchthreshold", "0.15"};
cvar_t	joy_yawthreshold = {"joyyawthreshold", "0.15"};
cvar_t	joy_forwardsensitivity = {"joyforwardsensitivity", "-1.0"};
cvar_t	joy_sidesensitivity = {"joysidesensitivity", "-1.0"};
cvar_t	joy_pitchsensitivity = {"joypitchsensitivity", "1.0"};
cvar_t	joy_yawsensitivity = {"joyyawsensitivity", "-1.0"};

qboolean	joy_advancedinit;


byte	scantokey[128] = {	0,K_ESCAPE,'1','2','3','4','5','6',	  				/* 00Å`07 */
				'7','8','9','0','-','^','\\',K_BACKSPACE,  				/* 08Å`0F */
				K_TAB,'q','w','e','r','t','y','u',    					/* 10Å`17 */
				'i','o','p','@','[',K_ENTER,'a','s', 	  				/* 18Å`1F */
				'd','f','g','h','j','k','l',';',	  				/* 20Å`27 */
				':',']','z','x','c','v','b','n',	  				/* 28Å`2F */
				'm',',','.','/','_',K_SPACE,K_HOME,K_DEL,				/* 30Å`37 */
				K_PGUP,K_PGDN,0,K_LEFTARROW,K_UPARROW,K_RIGHTARROW,K_DOWNARROW,0,	/* 38Å`3F */
				0,0,0,0,0,0,0,0,							/* 40Å`47 */
				0,0,0,0,0,0,0,0,							/* 48Å`4F */
				0,0,0,0,0,0,0,0,							/* 50Å`57 */
				0,0,0,0,0,0,K_INS,0,							/* 58Å`5F */
				0,0,0,K_F1,K_F2,K_F3,K_F4,K_F5,	  					/* 60Å`67 */
				K_F6,K_F7,K_F8,K_F9,K_F10,0,0,0,	  				/* 68Å`6F */
				K_SHIFT,K_CTRL,K_ALT,0,0,0,0,0,	 	 				/* 70Å`77 */
				0,0,0,0,0,0,0,0		  						/* 78Å`7F */
};

static byte oldkeystate[128];


void Joy_AdvancedUpdate_f (void);


void IN_Init (void)
{
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_advanced);
	Cvar_RegisterVariable (&joy_advaxisx);
	Cvar_RegisterVariable (&joy_advaxisy);
	Cvar_RegisterVariable (&joy_advaxisz);
	Cvar_RegisterVariable (&joy_advaxisr);
	Cvar_RegisterVariable (&joy_advaxisu);
	Cvar_RegisterVariable (&joy_advaxisv);
	Cvar_RegisterVariable (&joy_forwardthreshold);
	Cvar_RegisterVariable (&joy_sidethreshold);
	Cvar_RegisterVariable (&joy_pitchthreshold);
	Cvar_RegisterVariable (&joy_yawthreshold);
	Cvar_RegisterVariable (&joy_forwardsensitivity);
	Cvar_RegisterVariable (&joy_sidesensitivity);
	Cvar_RegisterVariable (&joy_pitchsensitivity);
	Cvar_RegisterVariable (&joy_yawsensitivity);

	Cmd_AddCommand ("joyadvancedupdate", Joy_AdvancedUpdate_f);

	memset(oldkeystate, 0, 128);

	joy_advancedinit = false;
}

void IN_Shutdown (void)
{
}

void IN_Commands (void)
{
	unsigned char tempkey[128];
	register byte i, key;
// get key events
	i = 0;
	do
	{
		key = BITSNS(i);
		tempkey[(i * 8)    ] = (key & 1) ? true : false;
		tempkey[(i * 8) + 1] = (key & 2) ? true : false;
		tempkey[(i * 8) + 2] = (key & 4) ? true : false;
		tempkey[(i * 8) + 3] = (key & 8) ? true : false;
		tempkey[(i * 8) + 4] = (key &16) ? true : false;
		tempkey[(i * 8) + 5] = (key &32) ? true : false;
		tempkey[(i * 8) + 6] = (key &64) ? true : false;
		tempkey[(i * 8) + 7] = (key&128) ? true : false;
	} while(++i < 16);

	i = 0;
	do
	{
		if(tempkey[i] != oldkeystate[i])
		{
			Key_Event(scantokey[i],tempkey[i]);
		}
	} while(++i < 128);

	memcpy(oldkeystate,tempkey,128);
}

void IN_MouseMove (usercmd_t *cmd)
{
	int		l, r, ret_ms;
	signed char	x, y;

	ret_ms = MS_GETDT();

	x = (ret_ms >> 24) & 0xff;
	y = (ret_ms >> 16) & 0xff;
	l = (ret_ms >>  8) & 0xff;
	r = (ret_ms	 ) & 0xff;

	Key_Event (K_MOUSE1, (l == 0xff) ? true : false);
	Key_Event (K_MOUSE2, (r == 0xff) ? true : false);

	mouse_x = (float)x * 8.0;
	mouse_y = (float)y * 8.0;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

	// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (in_mlook.state & 1)
		V_StopPitchDrift ();

	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

void Joy_AdvancedUpdate_f (void)
{

}

// Adapted from Quake's in_win.c, since in_dos.c only supports discrete joystick inputs.
void IN_JoyMove (usercmd_t *cmd)
{


}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove (cmd);
	IN_JoyMove (cmd);
}

#pragma GCC pop_options
