#pragma once

#include "inc.h"

#include <memory.h>

class CSDLDev
{
public:
	CSDLDev(){}

	static bool init()
	{
		// init sdl audio
		if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
		{
			return false;
		}

		return true;
	}

public:
	const char * getEnvChannels()
	{
		return SDL_getenv("SDL_AUDIO_CHANNELS");
	}

	bool open(SDL_AudioSpec& desired, SDL_AudioSpec& obtained)
	{
		return SDL_OpenAudio(&desired, &obtained)>=0;
	}

	void pause(bool bPause)
	{
		SDL_PauseAudio(bPause?1:0);
	}

	void addBuff(Uint8 * dst, const Uint8 * src, Uint32 len, Uint8 volume)
	{
		memset(dst, 0, len);
		SDL_MixAudio(dst, src, len, volume*SDL_MIX_MAXVOLUME/255);
	}

	void lock(bool bLock)
	{
		if (bLock)
		{
			SDL_LockAudio();
		}
		else
		{
			SDL_UnlockAudio();
		}
	}
	
	void close()
	{
		SDL_CloseAudio();
	}
};
