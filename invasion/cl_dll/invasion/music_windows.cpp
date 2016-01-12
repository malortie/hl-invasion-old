
#include <stdio.h>
#include "fmod_helpers.h"

#include <Windows.h>

int FMOD_Init(char* module, FMOD* pFMOD)
{
	if (!pFMOD)
		return 0;

	pFMOD->m_module = LoadLibrary(module);

	if (pFMOD->m_module == NULL)
		return 0;

	(FARPROC&)pFMOD->FSOUND_INIT = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Init@12");
	(FARPROC&)pFMOD->FSOUND_CLOSE = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Close@0");
	(FARPROC&)pFMOD->FSOUND_STREAM_OPENFILE = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Stream_OpenFile@12");
	(FARPROC&)pFMOD->FSOUND_STREAM_CLOSE = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Stream_Close@4");
	(FARPROC&)pFMOD->FSOUND_STREAM_PLAY = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Stream_Play@8");
	(FARPROC&)pFMOD->FSOUND_STREAM_ENDCALLBACK = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Stream_SetEndCallback@12");
	(FARPROC&)pFMOD->FSOUND_STREAM_STOP = GetProcAddress((HMODULE)pFMOD->m_module, "_FSOUND_Stream_Stop@4");

	if (!(pFMOD->FSOUND_INIT && pFMOD->FSOUND_CLOSE && pFMOD->FSOUND_STREAM_OPENFILE && pFMOD->FSOUND_STREAM_CLOSE && pFMOD->FSOUND_STREAM_PLAY && pFMOD->FSOUND_STREAM_ENDCALLBACK))
	{
		FreeLibrary((HMODULE)pFMOD->m_module);
		return 0;
	}

	if (!pFMOD->FSOUND_INIT(44100, 1, 0))
		return 0;

	return 1;
}

int FMOD_Shutdown(FMOD* pFMOD)
{
	if (pFMOD)
	{
		// FreeLibrary((HMODULE)pFMOD->m_module);
		pFMOD->m_module = NULL;

		return 1;
	}

	return 0;
}