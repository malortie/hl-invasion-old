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

#include "hud.h"
#include "cl_util.h"

#include "clientmusic.h"

CMusic g_MusicPlayer;

CMusic::CMusic()
{
	m_bInit = FALSE;
	m_IsPlaying = FALSE; 
	m_bIsRawSoundFile = FALSE;
	m_pTrack = NULL;
};

//---------------------------------------------------------
// initialisation

void CMusic :: Init ( void )
{
	m_bInit = TRUE;
}




//---------------------------------------------------------
// lecture d'un fichier audio


void CMusic :: OpenFile ( const char *filename, int repeat, int duration )
{
	// Raw sound file .i.e MP3
	if (repeat == 1 && duration == -1)
	{
		// This is a raw sound file .i.e MP3/OGG.
		m_bIsRawSoundFile = TRUE;
	}
	else
	{
		// This is a .txt file containing tracks to parse.
		m_bIsRawSoundFile = FALSE;
	}

	audiofile_t *p = NULL;
	p = new audiofile_t;

	sprintf(p->name, filename);
	p->repeat	= repeat;
	p->duration = duration;
	p->next		= m_pTrack;

	m_pTrack	= p;
}



//---------------------------------------------------------
// lecture d'une liste de fichiers audio


void CMusic :: OpenList ( const char *filename )
{
	
	// ouverture du fichier texte

	FILE *myfile = fopen ( filename, "r" );

	if ( myfile == NULL )
	{
		gEngfuncs.Con_Printf("\\\nMUSICPLAYER : impossible d'ouvrir %s\n\\\n", filename);
		return;
	}

	// enregistrement des morceaux dans la liste

	int total = 0;

	if ( fscanf ( myfile, "%i", &total ) != EOF )
	{
		for (int i = 0; i < total; i++)
		{
			char	ctitle[128];
			int		irepeat;
			int		iduration;

			// lecture du titre

			if (fscanf(myfile, "%s", ctitle) != EOF)
			{
				if (fscanf(myfile, "%i", &irepeat) != EOF)
				{
					if (fscanf(myfile, "%i", &iduration) != EOF)
						OpenFile(ctitle, irepeat, iduration);
				}
				else
					break;
			}
			else
				break;
		}
	}

	// fermeture du fichier texte

	fclose ( myfile );
}

//---------------------------------------------------------
// lecture


void CMusic::Play(void)
{
	if (m_IsPlaying == TRUE)
		return;

	// recherche du premier morceau de la liste

	audiofile_t *p = NULL;
	p = m_pTrack;

	while (p != NULL)
	{
		if (p->next == NULL)
			break;
		else
			p = p->next;
	}

	if (p == NULL)
	{
		gEngfuncs.Con_Printf("\\\nMUSICPLAYER : aucun morceau dans la liste\n\\\n");
		return;
	}

	// Convert MP3 path to media path and fix the sound name.
	if (m_bIsRawSoundFile)
	{
		char file[64];
		int len = strlen(p->name) - 1;

		// Start at the end.
		char* ptr = p->name + len;
		
		// Extract sound file name and extension.
		while (ptr != p->name && *ptr != '/' && *ptr != '\\')
			ptr--;

		ptr++;

		assert((*ptr != '/' && *ptr != '\\') == TRUE);

		// Format new file name and path.
		sprintf(file, "media/%s", ptr);

		// Erase everything in old file name and
		// copy new file path to the current track to be played.
		memset(p->name, 0, sizeof(p->name));
		strncpy(p->name, file, sizeof(file));
	}

	// Launch MP3 player.
	char cmd[128];
	sprintf(cmd, "mp3 play %s\n", p->name);

	if (gEngfuncs.pfnClientCmd(cmd))
	{
		// Delete the track since this is a single file.
		if (m_bIsRawSoundFile)
		{
			audiofile_t *p = NULL;

			while (m_pTrack != NULL)
			{
				p = m_pTrack;
				m_pTrack = p->next;
				delete p;
			}
		}
		else
		{
			// Register the sound duration and let know that we are playing.
			m_flDuration = gEngfuncs.GetClientTime() + p->duration;

			m_IsPlaying = TRUE;
		}
	}
	else
	{
		gEngfuncs.Con_Printf("\\\nMUSICPLAYER : %s : fichier introuvable\n\\\n", p->name);
	}
}


void CMusic :: Stop ( void )
{
	if ( m_IsPlaying == TRUE )
	{
		m_IsPlaying = FALSE;

		m_flDuration = 0;

		// Send client command to any currently playing MP3.
		gEngfuncs.pfnClientCmd( "mp3 stop\n" );
	}
}


void CMusic :: Reset ( void )
{
	//réinitialisation du lecteur

	Stop ();

	audiofile_t *p = NULL;
	
	while ( m_pTrack != NULL )
	{
		p = m_pTrack;
		m_pTrack = p->next;
		delete p;
	}
}

void CMusic::Update(void)
{
	// Do not update if not playing.
	if (!m_IsPlaying)
		return;

	// Do not move to next track unless finished playing.
	if (gEngfuncs.GetClientTime() <= m_flDuration)
		return;

	// fin du morceau

	Stop();

	// recherche du premier morceau de la liste

	audiofile_t *p = NULL;
	p = m_pTrack;

	while (p != NULL)
	{
		if (p->next == NULL)
			break;
		else
			p = p->next;
	}

	if (p == NULL)
	{
		gEngfuncs.Con_Printf("\\\nMUSICPLAYER : aucun morceau dans la liste\n\\\n");
		return;
	}

	// décrémentation de la répétition

	p->repeat--;

	// suppression des morceaux dont la répétition est terminée

	if (p->repeat < 1)
	{
		if (m_pTrack == p)
		{
			delete m_pTrack;
			m_pTrack = NULL;
		}
		else
		{
			audiofile_t *q = NULL;
			q = m_pTrack;

			while (q->next != p)
				q = q->next;

			delete q->next;
			q->next = NULL;
		}
	}

	// fermeture du lecteur si la liste est vide

	if (m_pTrack == NULL)
	{
		Reset();
	}

	// lancement du morceau suivant

	else
	{
		Play();
	}
}