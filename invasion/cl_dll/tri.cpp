//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

#include "particleman.h"
#include "tri.h"
extern IParticleMan *g_pParticleMan;

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void CL_DLLEXPORT HUD_DrawNormalTriangles( void )
{
//	RecClDrawNormalTriangles();

#if defined ( HLINVASION_CLIENT_DLL )
	// dessin du brouillard
	gHUD.m_Fog.DrawFog();
#endif

	gHUD.m_Spectator.DrawOverview();
}

#if defined( _TFC )
void RunEventList( void );
#endif

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void CL_DLLEXPORT HUD_DrawTransparentTriangles( void )
{
//	RecClDrawTransparentTriangles();

#if defined( _TFC )
	RunEventList();
#endif

#if defined ( HLINVASION_CLIENT_DLL )
	gHUD.m_Particules.DrawAll();	// affichage des particules et des decals
	gHUD.m_LFlammes.DrawFlammes();	// lance flammes
	gHUD.m_Briquet.DrawFlamme();	// briquet
	gHUD.m_LensFlare.DrawLight();	// lensflare
#endif // defined ( HLINVASION_CLIENT_DLL )

	if ( g_pParticleMan )
		 g_pParticleMan->Update();
}
