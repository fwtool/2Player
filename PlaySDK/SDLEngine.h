#pragma once

#include "SDLDev.h"

#include <functional>
using namespace std;

using CB_SDLStream = function<int(uint8_t *stream, unsigned int SDL_AudioBufSize, Uint8*& lpData)>;

struct tagSDLDevInfo
{
	Uint8 channels = 0;

	int freq = 0;

	int64_t audioDstChannelLayout = 0;

	SDL_AudioFormat audioDeviceFormat = AUDIO_F32SYS;  // audio device sample format
};

class CSDLEngine
{
public:
	CSDLEngine(){}

private:
	CSDLDev m_SDLDev;

	SDL_AudioSpec m_wantSpec;

	SDL_AudioSpec m_spec;

	tagSDLDevInfo m_DevInfo;
	
	CB_SDLStream m_cb;
	
	uint8_t m_volume = 100;

public:
	const tagSDLDevInfo& getDevInfo()
	{
		return m_DevInfo;
	}

	uint8_t getVolume()
	{
		return m_volume;
	}

	void setVolume(uint8_t volume)
	{
		m_volume = volume;
	}

	int64_t getChannelLayout();

	bool open(Uint8 wantedNbChannels, int sample_rate, Uint16 samples, int64_t audioDstChannelLayout);

	void start(const CB_SDLStream& cb);

	void pause(bool bPause)
	{
		m_SDLDev.pause(bPause);
	}

	void close();

private:
	static void audioCallback(void *userdata, uint8_t *stream, int SDL_AudioBufSize);
	void _audioCallback(uint8_t *stream, int SDL_AudioBufSize);
};
