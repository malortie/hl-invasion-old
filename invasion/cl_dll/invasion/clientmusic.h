//---------------------------------------------------------
//---------------------------------------------------------
//-														---
//-					music.h							---
//-														---
//---------------------------------------------------------
//---------------------------------------------------------
//-	par JujU									-----------
//-		julien.lecorre@free.fr					-----------
//---------------------------------------------------------
//- fichier d'en tête du lecteur mp3 pour mod HL		---
//---------------------------------------------------------
//-														---
//- compatible avec la version 3.6.1 de fmod.dll		---
//-		http://www.fmod.org/							---
//-														---
//---------------------------------------------------------



#ifndef MUSIC_H
#define MUSIC_H


#include "fmod_helpers.h"

//---------------------------------------------------------
// defines

#define	MUSIC_AUDIO_FILE		1
#define MUSIC_LIST_FILE			0

#define FMOD_DLL_PATH			"invasion\\fmod.dll"



//---------------------------------------------------------
// structure fichier audio

struct audiofile_t
{
	char name [128];
	int repeat;
	float duration;
	audiofile_t *next;
};

//---------------------------------------------------------
// classe du lecteur


class CMusic
{
public:

	// fonctions de lecture

	void OpenFile			( const char *filename, int repeat, int duration );	// ouverture d'un simple fichier
	void OpenList			( const char *filename );						// ouverture d'un fichier texte contenant les fichiers à lire

	void Init				( void );		// initialisation

	void Play				( void );		// lecture
	void Stop				( void );		// arrêt
	void Reset				( void );		// fermeture

	void Update				( void );

	// variables

	FSOUND_STREAM *m_fsound;				// handle du fichier en cours de lecture

	int m_IsPlaying;						// témoin de lecture
	int m_bInit;							// témoin d'inititalisation

	audiofile_t *m_pTrack;					// morceaux à jouer 
	float m_flDuration;
	int	m_bIsRawSoundFile;

	// constructeur / destructeur

	CMusic();
	~CMusic ()	{};

	// fonctions importées de la dll fmod
	FMOD		m_FMOD;
};

extern CMusic g_MusicPlayer;



#endif // MUSIC_H