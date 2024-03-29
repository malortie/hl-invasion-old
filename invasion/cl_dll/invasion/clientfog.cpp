/****************************************************************
*																*
*				clientfog.cpp									*
*																*
*				par Julien										*
*																*
****************************************************************/

// code de la partie client de l'effet de brouillard


//inclusions

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "parsemsg.h"
#include "triangleapi.h"

#define	FOG_DISTANCE_INFINIE		4096

#define FOG_SERVER_MAX_DENSITY		100
#define FOG_CLIENT_MAX_DENSITY		0.01f

//------------------------------------
//
// d�claration du message :
// gmsgFog

DECLARE_MESSAGE(m_Fog, Fog );


//------------------------------------
//
// gestion des messages serveur


int CHudFog::MsgFunc_Fog( const char *pszName, int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );

	int active	= READ_BYTE();
	maxfadetime	= READ_COORD();

	if ( maxfadetime > 0 )
		Fade = 1;

	bActive = active;
	fadetime = 0;

	mindist		= READ_COORD();
	maxdist		= READ_COORD();

	fogcolor.x	= READ_COORD();
	fogcolor.y	= READ_COORD();
	fogcolor.z	= READ_COORD();

	m_fogdensity = ConvertFogDensityFromServerToClient(READ_SHORT());

	m_iFlags |= HUD_ACTIVE;
	return 1;
}


//------------------------------------
//
// rafraichissement de l'affichage


int CHudFog :: Draw	( float flTime )
{
	if ( Fade == 1 )
	{
		fadetime = min ( maxfadetime, fadetime + gHUD.m_flTimeDelta );

		if ( fadetime >= maxfadetime )
			Fade = 0;
	}

	return 1;
}

void CHudFog :: DrawFog ( void )
{
	if ( Fade == 0 && m_iFlags & HUD_ACTIVE)
	{
		if ( bActive == 0 )
		{
			gEngfuncs.pTriAPI->FogParams(0, 0);
			gEngfuncs.pTriAPI->Fog ( fogcolor, mindist, maxdist, 0 );
			m_iFlags &= ~HUD_ACTIVE;
		}
		else
		{
			gEngfuncs.pTriAPI->FogParams(m_fogdensity, 0);
			gEngfuncs.pTriAPI->Fog(fogcolor, mindist, maxdist, 1);
		}
		return;
	}

	else if ( !(m_iFlags & HUD_ACTIVE) )
		return;

	float fldist;

	if ( Fade == 1 && bActive == 1 )
		fldist = FOG_DISTANCE_INFINIE * ( maxfadetime - fadetime ) / maxfadetime;

	else if ( Fade == 1 && bActive == 0 )
		fldist = FOG_DISTANCE_INFINIE * fadetime / maxfadetime;

	gEngfuncs.pTriAPI->FogParams(m_fogdensity, 0);
	gEngfuncs.pTriAPI->Fog ( fogcolor, mindist + fldist, maxdist + fldist, 1 );
}



//------------------------------------
//
// initialisation au chargement de la dll

int CHudFog :: Init( void )
{
	bActive = 0;
	Fade = 0;

	HOOK_MESSAGE( Fog );

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return 1;
}


//------------------------------------
//
// initialisation apr�s le chargement


int CHudFog :: VidInit( void )
{
	bActive = 0;
	Fade = 0;

	HOOK_MESSAGE( Fog );

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return 1;
}

inline float clamp(float val, float minVal, float maxVal)
{
	if (maxVal < minVal)
		return maxVal;
	else if (val < minVal)
		return minVal;
	else if (val > maxVal)
		return maxVal;
	else
		return val;
}

inline float RemapValClamped(float val, float A, float B, float C, float D)
{
	if (A == B)
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	cVal = clamp(cVal, 0.0f, 1.0f);

	return C + (D - C) * cVal;
}

float CHudFog::ConvertFogDensityFromServerToClient(short serverFogDensity)
{
	return RemapValClamped(serverFogDensity, 0, FOG_SERVER_MAX_DENSITY, 0, FOG_CLIENT_MAX_DENSITY);
}
