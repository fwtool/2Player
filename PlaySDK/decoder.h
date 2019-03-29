#ifndef DECODER_H
#define DECODER_H

#include "audiodecoder.h"

enum class E_DecodeState {
	DS_STOP,
	DS_PAUSE,
	DS_DECODING,
};

enum class E_DecoderRetCode
{
	DRC_Success = 0
	, DRC_Fail
	, DRC_OpenFail
	, DRC_NoAudioStream
	, DRC_InvalidAudioStream
};

class Decoder
{
public:
	Decoder() {}

    //static bool initFFmpeg();

private:
	E_DecodeState m_decodeState = E_DecodeState::DS_STOP;

	tagDecodeCtrl m_DecodeCtrl;

	AudioDecoder m_audioDecoder;

	AVFormatContext *m_pFormatCtx = NULL;

	int m_audioIndex = 0;

	int64_t m_duration = 0;

	int64_t m_seekPos = -1;

public:
	uint64_t getClock()
	{
		return m_DecodeCtrl.m_clock;
	}
	
	void seek(uint64_t pos);

	uint8_t getVolume();
    void setVolume(uint8_t volume);

	int64_t check(const char *pszFile);

	int64_t open(const char *pszFile, const CB_DecodeFinished& cbPlayFinished);

	void start();

	void pause();

	void stop();

private:
	E_DecoderRetCode _open(const char *pszFile);

	void _freeFormatCtx();

	void _clearData();
};

#endif // DECODER_H
