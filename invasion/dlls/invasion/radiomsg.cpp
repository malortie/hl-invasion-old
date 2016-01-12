//-------------------------------------------------
//-												---
//-			radiomsg.cpp						---
//-												---
//-------------------------------------------------
//			par Julien		-----------------------
//-------------------------------------------------
//- code serveur de la radio du hud   -------------
//-------------------------------------------------


//----------------------------------------
// inclusions

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"

extern int gmsgRadioMsg;

int	GetRadiomsgText ( int iszMessage )
{
	return ALLOC_STRING( STRING( iszMessage ) );
}

//-----------------------------------------------------------------
//
//

class CRadiomsg : public CPointEntity
{
public:
	void	Spawn		( void );
	void	Precache	( void );

	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue( KeyValueData *pkvd );

	int		m_iszSentence;
	int		m_iszMessage;
	int		m_iszText;
	int		m_iHead;

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
};

LINK_ENTITY_TO_CLASS( trigger_radio_message, CRadiomsg );


TYPEDESCRIPTION	CRadiomsg::m_SaveData[] = 
{
	DEFINE_FIELD( CRadiomsg, m_iszMessage, FIELD_STRING ),
	DEFINE_FIELD( CRadiomsg, m_iszSentence, FIELD_STRING ),
	DEFINE_FIELD( CRadiomsg, m_iszText, FIELD_STRING ),
	DEFINE_FIELD( CRadiomsg, m_iHead, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE( CRadiomsg, CPointEntity );



void CRadiomsg::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "radiomsg"))
	{
		m_iszMessage = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iHead = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}


	else
		CPointEntity::KeyValue( pkvd );
}


void CRadiomsg :: Spawn( void )
{
	Precache ( );

	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;

}


void CRadiomsg :: Precache ( void )
{
	m_iszText = GetRadiomsgText ( m_iszMessage );
}


void CRadiomsg :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = UTIL_FindEntityByClassname ( NULL,"player" );
	if (!pPlayer)
		return;

	static char txt[256];
	memset(txt, 0, sizeof(txt));

	sprintf(txt, STRING(m_iszText));

	ALERT(at_console, "Radio message: %s\n", txt);

	MESSAGE_BEGIN( MSG_ONE, gmsgRadioMsg, NULL, pPlayer->pev );

		WRITE_COORD ( gpGlobals->time );
		WRITE_LONG	( m_iHead );
		WRITE_STRING( txt );

	MESSAGE_END();

	if ( FStringNull ( m_iszSentence ) )
		return;

//	EMIT_SOUND_SUIT(pPlayer->edict(), STRING(m_iszSentence) );

	EMIT_SOUND_DYN(pPlayer->edict(), CHAN_STATIC, STRING(m_iszSentence), 1.0, ATTN_NORM, 0, 100);
}