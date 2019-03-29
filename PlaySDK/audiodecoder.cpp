
#include "audiodecoder.h"

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512

/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

int AudioDecoder::open(AVStream& stream, const CB_DecodeFinished& cbDecodeFinished)
{
	m_codecCtx = avcodec_alloc_context3(NULL);
	if (NULL == m_codecCtx)
	{
		return -1;
	}

	auto fnInitCodecCtx = [&](){	
		stream.discard = AVDISCARD_DEFAULT;
		if (avcodec_parameters_to_context(m_codecCtx, stream.codecpar) < 0)
		{
			return false;
		}

		if (m_codecCtx->channels <= 0 || m_codecCtx->sample_rate <= 0)
		{
			//QDebug << "Invalid channels or sample_rate";
			return false;
		}

		/* find audio decoder */
		AVCodec *codec = avcodec_find_decoder(m_codecCtx->codec_id);
		if (NULL == codec)
		{
			//QDebug << "Audio decoder not found.";
			return false;
		}
		/* open audio decoder */
		if (avcodec_open2(m_codecCtx, codec, NULL) < 0)
		{
			//QDebug << "Could not open audio decoder.";
			return false;
		}

		return true;
	};

	if (!fnInitCodecCtx())
	{
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = NULL;
		return -1;
	}

	int64_t audioDstChannelLayout = m_SDLEngine.getChannelLayout();
    if (NULL == audioDstChannelLayout || (m_codecCtx->channels != av_get_channel_layout_nb_channels(audioDstChannelLayout)))
	{
		audioDstChannelLayout = av_get_default_channel_layout(m_codecCtx->channels);
		audioDstChannelLayout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

	Uint16 samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(m_codecCtx->sample_rate / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	if (!m_SDLEngine.open(m_codecCtx->channels, m_codecCtx->sample_rate, samples, audioDstChannelLayout))
	{
		this->_clearData();
		return -1;
	}

	m_AudioInfo.channels = m_codecCtx->channels;

	m_AudioInfo.sample_rate = m_codecCtx->sample_rate;

	/* set sample format */
	uint8_t audioDepth = 0;
	auto fnGetSampleFormat = [&]() {
		switch (m_SDLEngine.getDevInfo().audioDeviceFormat) {
		case AUDIO_U8:
			audioDepth = 1;
			return AV_SAMPLE_FMT_U8;
		case AUDIO_S16SYS:
			audioDepth = 2;
			return AV_SAMPLE_FMT_S16;
		case AUDIO_S32SYS:
			audioDepth = 4;
			return AV_SAMPLE_FMT_S32;
		case AUDIO_F32SYS:
			audioDepth = 4;
			return AV_SAMPLE_FMT_FLT;
		default:
			audioDepth = 2;
			return AV_SAMPLE_FMT_S16;
		}
	};
	m_AudioInfo.audioDstFmt = fnGetSampleFormat();

	m_AudioInfo.bytesPerSec = m_AudioInfo.sample_rate * m_AudioInfo.channels * audioDepth;

	m_AudioInfo.timeBase = av_q2d(stream.time_base)*__1e6;

	m_cbDecodeFinished = cbDecodeFinished;

    return 0;
}

void AudioDecoder::start(tagDecodeCtrl& DecodeCtrl)
{
	m_SDLEngine.start([&](uint8_t *stream, unsigned int SDL_AudioBufSize, Uint8*& lpData) {
		if (DecodeCtrl.m_isStoping)
		{
			return -1;
		}

		if (DecodeCtrl.m_isSeek)
		{
			DecodeCtrl.m_isSeek = false;

			avcodec_flush_buffers(m_codecCtx);

			m_DecodeData.sendReturn = 0;

			m_DecodeData.audioBufSize = 0;

			return 0;
		}

		if (0 == m_DecodeData.audioBufSize)	/* no data in buffer */
		{
			bool bQueedEmpty = false;
			int32_t audioBufSize = _decodePacket(DecodeCtrl, bQueedEmpty);
			if (audioBufSize < 0 || (bQueedEmpty && DecodeCtrl.m_isReadFinished))
			{
				DecodeCtrl.m_isStoping = true;

				//SDL_Delay(100);

				if (m_cbDecodeFinished)
				{
					m_cbDecodeFinished();
				}

				return -1;
			}

			if (0 == audioBufSize)
			{
				/* if not decoded data, just output silence */
				//m_DecodeData.audioBufSize = 1024;
				return 0;
			}

			m_DecodeData.audioBufSize = audioBufSize;
		}

		int len = min(SDL_AudioBufSize, m_DecodeData.audioBufSize);
		m_DecodeData.audioBufSize -= len;

		if (NULL != m_DecodeData.audioBuf)
		{
			lpData = m_DecodeData.audioBuf;
			m_DecodeData.audioBuf += len;
		}

		return len;
	});
}

int32_t AudioDecoder::_decodePacket(tagDecodeCtrl& DecodeCtrl, bool& bQueedEmpty)
{
	m_DecodeData.audioBuf = nullptr;

	AVPacket& packet = m_DecodeData.packet;
	/* get new packet whiel last packet all has been resolved */
	if (m_DecodeData.sendReturn != AVERROR(EAGAIN))
	{
		if (!DecodeCtrl.m_packetQueue.dequeue(packet, false))
		{
			bQueedEmpty = true;

			return 0;
		}
	}

	/* while return -11 means packet have data not resolved,
	* this packet cannot be unref */
	m_DecodeData.sendReturn = avcodec_send_packet(m_codecCtx, &packet);
	if (m_DecodeData.sendReturn < 0 && m_DecodeData.sendReturn != AVERROR(EAGAIN) && m_DecodeData.sendReturn != AVERROR_EOF)
	{
		//QDebug << "Audio send to decoder failed, error code: " << sendReturn;'
		av_packet_unref(&packet);
		return m_DecodeData.sendReturn;
	}

	int32_t iRet = _receiveFrame(DecodeCtrl);
	if (iRet < 0)
	{
		av_packet_unref(&m_DecodeData.packet);
		return -1;
	}

	if (m_DecodeData.sendReturn != AVERROR(EAGAIN))
	{
		av_packet_unref(&m_DecodeData.packet);
	}

	return iRet;
}

int32_t AudioDecoder::_receiveFrame(tagDecodeCtrl& DecodeCtrl)
{
	AVFrame *frame = av_frame_alloc();
	if (NULL == frame)
	{
		//QDebug << "Decode audio frame alloc failed.";
		return -1;
	}

	int iRet = avcodec_receive_frame(m_codecCtx, frame);
	if (iRet < 0)
	{
		av_frame_free(&frame);

		//QDebug << "Audio frame decode failed, error code: " << ret;
		if (iRet == AVERROR(EAGAIN))
		{
			return 0;
		}
		
		return iRet;
	}

	int32_t audioBufSize = _convertFrame(*frame);

	if (AV_NOPTS_VALUE != frame->pts)
	{
		DecodeCtrl.m_clock = uint64_t(m_AudioInfo.timeBase * frame->pts);
	}
	DecodeCtrl.m_clock += uint64_t(audioBufSize*__1e6 / m_AudioInfo.bytesPerSec);

	av_frame_free(&frame);

	return audioBufSize;
}

int32_t AudioDecoder::_convertFrame(AVFrame& frame)
{
	/* get audio channels */
	int64_t inChannelLayout = (frame.channel_layout && frame.channels == av_get_channel_layout_nb_channels(frame.channel_layout)) ?
		frame.channel_layout : av_get_default_channel_layout(frame.channels);
	if (frame.format != m_DecodeData.audioSrcFmt ||
		inChannelLayout != m_DecodeData.audioSrcChannelLayout ||
		frame.sample_rate != m_DecodeData.audioSrcFreq ||
		!m_DecodeData.aCovertCtx)
	{
		if (m_DecodeData.aCovertCtx)
		{
			swr_free(&m_DecodeData.aCovertCtx);
			m_DecodeData.aCovertCtx = NULL;
		}

		/* init swr audio convert context */
		m_DecodeData.aCovertCtx = swr_alloc_set_opts(nullptr, m_SDLEngine.getDevInfo().audioDstChannelLayout, m_AudioInfo.audioDstFmt, m_SDLEngine.getDevInfo().freq,
			inChannelLayout, (AVSampleFormat)frame.format, frame.sample_rate, 0, NULL);
		if (NULL == m_DecodeData.aCovertCtx)
		{
			return -1;
		}

		if (swr_init(m_DecodeData.aCovertCtx) < 0)
		{
			swr_free(&m_DecodeData.aCovertCtx);
			m_DecodeData.aCovertCtx = NULL;

			return -1;
		}

		m_DecodeData.audioSrcFmt = (AVSampleFormat)frame.format;
		m_DecodeData.audioSrcChannelLayout = inChannelLayout;
		m_DecodeData.audioSrcFreq = frame.sample_rate;
	} 

	uint32_t audioBufSize = 0;
	if (m_DecodeData.aCovertCtx)
	{
		const uint8_t **in = (const uint8_t **)frame.extended_data;
		uint8_t *out[] = { m_DecodeData.swrAudioBuf };

		int outCount = sizeof(m_DecodeData.swrAudioBuf) / m_SDLEngine.getDevInfo().channels / av_get_bytes_per_sample(m_AudioInfo.audioDstFmt);

		int sampleSize = swr_convert(m_DecodeData.aCovertCtx, out, outCount, in, frame.nb_samples);
		if (sampleSize < 0)
		{
			//QDebug << "swr convert failed";
			return -1;
		}

		if (sampleSize == outCount) {
			//QDebug << "audio buffer is probably too small";
			if (swr_init(m_DecodeData.aCovertCtx) < 0) {
				swr_free(&m_DecodeData.aCovertCtx);
				m_DecodeData.aCovertCtx = NULL;
			}
		}

		m_DecodeData.audioBuf = m_DecodeData.swrAudioBuf;
		audioBufSize = sampleSize * m_SDLEngine.getDevInfo().channels * av_get_bytes_per_sample(m_AudioInfo.audioDstFmt);
	}
	else
	{
		m_DecodeData.audioBuf = frame.data[0];
		audioBufSize = av_samples_get_buffer_size(NULL, frame.channels, frame.nb_samples, static_cast<AVSampleFormat>(frame.format), 1);
	}

	return audioBufSize;
}

void AudioDecoder::close()
{
	m_SDLEngine.close();

	_clearData();
}

void AudioDecoder::_clearData()
{
	if (m_codecCtx)
	{
		avcodec_close(m_codecCtx);
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = NULL;
	}

	m_AudioInfo.init();

	m_DecodeData.init();
}
