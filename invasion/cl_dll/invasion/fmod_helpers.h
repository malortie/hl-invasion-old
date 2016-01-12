#ifndef FMOD_HELPERS_H
#define FMOD_HELPERS_H

#include "fmod.h"

#ifdef WIN32
#define FMOD_API	_stdcall
#else
#define FMOD_API	_cdecl
#endif

struct FMOD
{
	signed char		(FMOD_API * FSOUND_INIT)				(int mixrate, int maxsoftwarechannels, unsigned int flags);
	void			(FMOD_API * FSOUND_CLOSE)				(void);
	FSOUND_STREAM *	(FMOD_API * FSOUND_STREAM_OPENFILE)		(const char *filename, unsigned int mode, int memlength);
	signed char		(FMOD_API * FSOUND_STREAM_CLOSE)		(FSOUND_STREAM *stream);
	int				(FMOD_API * FSOUND_STREAM_PLAY)			(int channel, FSOUND_STREAM *stream);
	signed char		(FMOD_API * FSOUND_STREAM_ENDCALLBACK)	(FSOUND_STREAM *stream, FSOUND_STREAMCALLBACK callback, int userdata);
	signed char     (FMOD_API * FSOUND_STREAM_STOP)			(FSOUND_STREAM *stream);

	void*		m_module;
};


#endif // FMOD_HELPERS_H