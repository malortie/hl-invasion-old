//---------------------------------------------------------
//---------------------------------------------------------
//-														---
//-					music.cpp							---
//-														---
//---------------------------------------------------------
//---------------------------------------------------------
//-	par JujU									-----------
//-		julien.lecorre@free.fr					-----------
//---------------------------------------------------------
//- code du lecteur mp3	pour mod HL				-----------
//---------------------------------------------------------
//-														---
//- compatible avec la version 3.6.1 de fmod.dll		---
//-		http://www.fmod.org/							---
//-														---
//---------------------------------------------------------


/*//---------------
mettre la dll fmod.dll dans le dossier half life.
changer l'adresse de la dll dans le #define FMOD_DLL_PATH
du fichier .h
attention, mettre des \\ et pas des \ dans l'adresse.
code à rajouter dans le fgd de votre mod : voir à la
fin de ce fichier.
composition des fichiers texte listes de fichiers audio :
voir à la fin de ce fichier.
*///---------------



//---------------------------------------------------------
// inclusions

#include "extdll.h"
#include "util.h"
#include "cbase.h"

extern int gmsgMusic;

//---------------------------------------------------------
// classe de l'entité dans les maps bsp



class CTriggerMusic : public CPointEntity
{
public:

	void	Spawn		( void );

	void	KeyValue	( KeyValueData *pkvd );
	void	Use			( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	Save	( CSave &save );
	virtual int	Restore	( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];


	string_t	m_iFileName;		// chemin du fichier
	int			m_iFileType;		// fichier texte ( liste ) ou fichier audio

};

LINK_ENTITY_TO_CLASS( trigger_music, CTriggerMusic );



TYPEDESCRIPTION CTriggerMusic::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerMusic, m_iFileType, FIELD_INTEGER ),
	DEFINE_FIELD( CTriggerMusic, m_iFileName, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CTriggerMusic, CPointEntity );



void CTriggerMusic :: Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}

void CTriggerMusic :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "filetype"))
	{
		m_iFileType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "filename"))
	{
		m_iFileName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

void CTriggerMusic :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pPlayer = NULL;
	pPlayer = UTIL_FindEntityByClassname(NULL, "player");

	MESSAGE_BEGIN( MSG_ONE, gmsgMusic, NULL, pPlayer->pev );
		WRITE_LONG(m_iFileType);
		WRITE_STRING(STRING(m_iFileName));
	MESSAGE_END();
}


/*//---------------
code à rajouter à la fin du le fgd de votre mod :
@PointClass base( Targetname ) = trigger_music : "Trigger Music"
[
	filetype(choices) : "Type de fichier" : 0 = 
	[
		0: "Liste de fichiers (*.txt)"
		1: "Fichier wav mp2 mp3 ogg raw"
	]
	filename(string) : "Nom (mod/dossier/fichier.extension)"
]
*///---------------


/*//---------------
composition des listes de fichiers audio
exemple : fichier music01.txt :
//
3
monmod/sound/mp3/music01_debut.mp3		1
monmod/sound/mp3/music01_boucle.mp3		3
monmod/sound/mp3/music01_fin.mp3		1
//
composition :
	- nombre total de morceaux différents à jouer ( ici 3 )
	- adresse du premier fichier musique à partir du répertoire Half-Life
	- nombre de lectures de ce fichier
	- adresse du deuxième fichier musique à partir du répertoire Half-Life
	- etc ...
*///---------------