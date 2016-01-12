/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// battery.cpp
//
// implementation of CHudBattery class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "clientmusic.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_Music, Music)

int CHudMusic::Init(void)
{
	HOOK_MESSAGE(Music);

	g_MusicPlayer.Init();

	m_iFlags = 0;

	gHUD.AddHudElem(this);

	return 1;
};


int CHudMusic::VidInit(void)
{
	g_MusicPlayer.Reset();

	m_iFlags = 0;

	return 1;
};

void CHudMusic::Reset(void)
{
	g_MusicPlayer.Reset();
}

void CHudMusic::Shutdown(void)
{
	g_MusicPlayer.Reset();
}

int CHudMusic::Draw(float flTime)
{
	if (g_MusicPlayer.m_IsPlaying)
	{
		g_MusicPlayer.Update();
	}

	return 1;
}

int CHudMusic::MsgFunc_Music(const char *pszName, int iSize, void *pbuf)
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ(pbuf, iSize);

	int fileType = READ_LONG();
	const char* filename = READ_STRING();

	if (g_MusicPlayer.m_IsPlaying == TRUE)
		return 1;

	if (fileType == MUSIC_AUDIO_FILE)
	{
		g_MusicPlayer.OpenFile(filename, 1, -1);
	}
	else
	{
		g_MusicPlayer.OpenList(filename);
	}

	g_MusicPlayer.Play();


	return 1;
}