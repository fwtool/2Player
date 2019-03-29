
#include "SDLEngine.h"

/*  soundtrack array use to adjust */
static int g_nextNbChannels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };

static int g_nextSampleRates[] = { 0, 44100, 48000, 96000, 192000 };

int64_t CSDLEngine::getChannelLayout()
{
	const char *env = m_SDLDev.getEnvChannels();
	if (NULL != env)
	{
		//QDebug << "SDL audio channels";
		m_DevInfo.audioDstChannelLayout = av_get_default_channel_layout(atoi(env));
	}

	return m_DevInfo.audioDstChannelLayout;
}

bool CSDLEngine::open(Uint8 wantedNbChannels, int sample_rate, Uint16 samples, int64_t audioDstChannelLayout)
{
	m_DevInfo.audioDstChannelLayout = audioDstChannelLayout;

	memset(&m_wantSpec, 0, sizeof(m_wantSpec));
	m_wantSpec.channels = wantedNbChannels;
	m_wantSpec.freq = sample_rate;
	m_wantSpec.format = m_DevInfo.audioDeviceFormat;
	m_wantSpec.silence = 0;
	m_wantSpec.samples = samples;
	m_wantSpec.callback = &CSDLEngine::audioCallback;
	m_wantSpec.userdata = this;

	int nextSampleRateIdx = FF_ARRAY_ELEMS(g_nextSampleRates) - 1;
	while (nextSampleRateIdx && g_nextSampleRates[nextSampleRateIdx] >= m_wantSpec.freq) {
		nextSampleRateIdx--;
	}

	/* This function opens the audio device with the desired parameters, placing
	* the actual hardware parameters in the structure pointed to spec.
	*/
	while (true)
	{
		while (!m_SDLDev.open(m_wantSpec, m_spec))
		{
			//QDebug << QString("SDL_OpenAudio (%1 channels, %2 Hz): %3")
			//.arg(m_wantSpec.channels).arg(m_wantSpec.freq).arg(SDL_GetError());
			m_wantSpec.channels = g_nextNbChannels[FFMIN(7, m_wantSpec.channels)];
			if (0 != m_wantSpec.channels)
			{
				if (nextSampleRateIdx <= 0)
				{
					return false;
				}

				m_wantSpec.freq = g_nextSampleRates[nextSampleRateIdx--];
				m_wantSpec.channels = wantedNbChannels;
				if (!m_wantSpec.freq)
				{
					//QDebug << "No more combinations to try, audio open failed";
					return false;
				}
			}
			m_DevInfo.audioDstChannelLayout = av_get_default_channel_layout(m_wantSpec.channels);
		}

		if (m_spec.format != m_DevInfo.audioDeviceFormat)
		{
			//QDebug << "SDL audio format: " << m_wantSpec.format << " is not supported"
			//<< ", set to advised audio format: " <<  spec.format;
			m_wantSpec.format = m_spec.format;
			m_DevInfo.audioDeviceFormat = m_spec.format;
			m_SDLDev.close();
		}
		else
		{
			break;
		}
	}

	if (m_spec.channels != m_wantSpec.channels)
	{
		m_DevInfo.audioDstChannelLayout = av_get_default_channel_layout(m_spec.channels);
		if (!m_DevInfo.audioDstChannelLayout)
		{
			//QDebug << "SDL advised channel count " << spec.channels << " is not supported!";
			return false;
		}
	}

	m_DevInfo.channels = m_spec.channels;
	m_DevInfo.freq = m_spec.freq;

	return true;
}

void CSDLEngine::start(const CB_SDLStream& cb)
{
	m_cb = cb;

	m_SDLDev.pause(false);
}

void CSDLEngine::audioCallback(void *userdata, uint8_t *stream, int SDL_AudioBufSize)
{
	CSDLEngine *pThis = (CSDLEngine *)userdata;
	pThis->_audioCallback(stream, SDL_AudioBufSize);
}

void CSDLEngine::_audioCallback(uint8_t *stream, int SDL_AudioBufSize)
{
    /* SDL_BufSize means audio play buffer left size
     * while it greater than 0, means counld fill data to it
     */
	while (SDL_AudioBufSize > 0)
	{
		Uint8* lpData = NULL;
		int len = m_cb(stream, SDL_AudioBufSize, lpData);
		if (len > 0)
		{
			if (NULL != lpData)
			{
				m_SDLDev.addBuff(stream, lpData, len, m_volume);
			}

			SDL_AudioBufSize -= len;
			stream += len;

			continue;
		}

		SDL_Delay(10);

		if (len < 0)
		{
			return;
		}
	}
}

void CSDLEngine::close()
{
	m_SDLDev.close();
}
