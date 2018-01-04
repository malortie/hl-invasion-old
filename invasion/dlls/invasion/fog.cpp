//---------------------------------------------------------
//---------------------------------------------------------
//-														---
//-					fog.cpp								---
//-														---
//---------------------------------------------------------
//---------------------------------------------------------
//- partie serveur de l'effet de brouillard				---
//---------------------------------------------------------


//---------------------------------------------------------
// inclusions

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "fog.h"

extern int gmsgFog;

int	g_bFogUpdate;


//---------------------------------------------------------
// classe de l'entité dans les maps bsp

LINK_ENTITY_TO_CLASS( trigger_fog, CTriggerFog );

TYPEDESCRIPTION CTriggerFog::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerFog, m_flminDist, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerFog, m_flmaxDist, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerFog, m_flFadeInTime, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerFog, m_flFadeOutTime, FIELD_FLOAT ),
	DEFINE_FIELD( CTriggerFog, m_bActive, FIELD_INTEGER ),
	DEFINE_FIELD( CTriggerFog, m_fogdensity, FIELD_SHORT ),
};
IMPLEMENT_SAVERESTORE(CTriggerFog, CPointEntity);

void CTriggerFog :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

	m_bActive = 0;

	g_bFogUpdate = 0;
	m_fogdensity = 0;
}

void CTriggerFog :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "mindist"))
	{
		m_flminDist = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maxdist"))
	{
		m_flmaxDist = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_flFadeInTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_flFadeOutTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

void CTriggerFog :: Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// value = 0 : activation
	// value = 18686 : update du client

	if ( value != 18686 )
	{
		CBaseEntity *pPlayer = NULL;
		pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

		MESSAGE_BEGIN( MSG_ONE, gmsgFog, NULL, pPlayer->pev );

			if ( m_bActive == 0 )
			{
				WRITE_BYTE ( 1 );				// appartion
				WRITE_COORD ( m_flFadeInTime );		// temps
			}
			else
			{
				WRITE_BYTE ( 0 );				// disparition
				WRITE_COORD ( m_flFadeOutTime );	// temps
			}

			WRITE_COORD ( m_flminDist );				// distances
			WRITE_COORD ( m_flmaxDist );

			WRITE_COORD ( pev->rendercolor.x );			// couleur
			WRITE_COORD ( pev->rendercolor.y );
			WRITE_COORD ( pev->rendercolor.z );

			WRITE_SHORT(m_fogdensity);

		MESSAGE_END();

		m_bActive = m_bActive == 1 ? 0 : 1;
	}

	else if ( value == 18686 )
	{
		CBaseEntity *pPlayer = NULL;
		pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

		MESSAGE_BEGIN( MSG_ONE, gmsgFog, NULL, pPlayer->pev );

			WRITE_BYTE ( m_bActive );
			WRITE_COORD ( 0 );
			WRITE_COORD ( m_flminDist );
			WRITE_COORD ( m_flmaxDist );
			WRITE_COORD ( pev->rendercolor.x );
			WRITE_COORD ( pev->rendercolor.y );
			WRITE_COORD ( pev->rendercolor.z );

			WRITE_SHORT( m_fogdensity );

		MESSAGE_END();
	}
}

void CTriggerFog::Activate()
{
	CPointEntity::Activate();

	ApplySteamPipeFogDensityFix();

	if (m_bActive)
		g_bFogUpdate = 1;
}

void CTriggerFog::ApplySteamPipeFogDensityFix()
{
	if (FStrEq(STRING(gpGlobals->mapname), "l4m5"))
	{
		if (FStrEq(STRING(pev->targetname), "fog_gaz_fuck"))
			m_fogdensity = 25;
		else if (FStrEq(STRING(pev->targetname), "fog_gaz_fuck2"))
			m_fogdensity = 8;
	}
	else if (FStrEq(STRING(gpGlobals->mapname), "l4m6") && FStrEq(STRING(pev->targetname), "new_fogue"))
		m_fogdensity = 8;
	else if (FStrEq(STRING(gpGlobals->mapname), "l4m7") && FStrEq(STRING(pev->targetname), "autofog2"))
		m_fogdensity = 6;
	else if (FStrEq(STRING(gpGlobals->mapname), "l5m1") && FStrEq(STRING(pev->targetname), "fuckfog"))
		m_fogdensity = 8;
	else if (FStrEq(STRING(gpGlobals->mapname), "l5m2"))
	{
		if (FStrEq(STRING(pev->targetname), "fuckfog"))
			m_fogdensity = 6;
		else if (FStrEq(STRING(pev->targetname), "newfog"))
			m_fogdensity = 6;
	}
	else if (FStrEq(STRING(gpGlobals->mapname), "l5m3"))
		m_fogdensity = 6;
	else
		m_fogdensity = 0;
}