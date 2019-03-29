#pragma once

#include "SDLEngine.h"

#include "avpacketqueue.h"

#define __1e6 ((uint32_t)1e6)

using CB_DecodeFinished = function<void()>;

struct tagAudioInfo
{
	tagAudioInfo()
	{
		init();
	}

	void init()
	{
		channels = 0;

		sample_rate = 0;

		audioDstFmt = AV_SAMPLE_FMT_NONE;

		bytesPerSec = 0;

		timeBase = 0;
	}

	int channels;
	int sample_rate;

	AVSampleFormat audioDstFmt;   // audio decode sample format

	int bytesPerSec;

	double timeBase;
};

struct tagDecodeData
{
	tagDecodeData()
	{
		init();
	}

	void init()
	{
		sendReturn = 0;

		audioBuf = nullptr;
		audioBufSize = 0;

		audioSrcFmt = AV_SAMPLE_FMT_NONE;
		audioSrcChannelLayout = 0;
		audioSrcFreq = 0;
	}

    int sendReturn = 0;

	AVPacket packet;

	SwrContext *aCovertCtx = NULL;
	DECLARE_ALIGNED(16, uint8_t, swrAudioBuf)[192000];

    uint8_t *audioBuf = NULL;
    uint32_t audioBufSize = 0;

	AVSampleFormat audioSrcFmt;
    int64_t audioSrcChannelLayout = 0;
    int audioSrcFreq = 0;
};

struct tagDecodeCtrl
{
	bool m_isReadFinished = false;

	bool m_isStoping = false;

	bool m_isSeek = false;

	AvPacketQueue m_packetQueue;
	
	uint64_t m_clock = 0;
};

class AudioDecoder
{
public:
	AudioDecoder() {}

private:
	CB_DecodeFinished m_cbDecodeFinished;

	AVCodecContext *m_codecCtx = NULL;          // audio codec context
	
	tagAudioInfo m_AudioInfo;

	CSDLEngine m_SDLEngine;

	tagDecodeData m_DecodeData;

public:
	uint8_t getVolume()
	{
		return m_SDLEngine.getVolume();
	}

	void setVolume(uint8_t volume)
	{
		m_SDLEngine.setVolume(volume);
	}

	int open(AVStream& stream, const CB_DecodeFinished& cbDecodeFinished);

	void start(tagDecodeCtrl& DecodeCtrl);

	void pause(bool bPause)
	{
		m_SDLEngine.pause(bPause);
	}

	void close();

private:
	int32_t _decodePacket(tagDecodeCtrl& DecodeCtrl, bool& bQueedEmpty);
    int32_t _receiveFrame(tagDecodeCtrl& DecodeCtrl);
	int32_t _convertFrame(AVFrame& frame);

	void _clearData();
};
