
#include "decoder.h"

/*bool Decoder::initFFmpeg()
{
	// av_log_set_level(AV_LOG_INFO);

    //avfilter_register_all();

	//av_register_all();

    // ffmpeg network init for rtsp
    //if (avformat_network_init()) {
    //    return false;
    //}

    return true;
}*/

#define __sleep(ms) this_thread::sleep_for(chrono::milliseconds(ms))

uint8_t Decoder::getVolume()
{
	return m_audioDecoder.getVolume();
}

void Decoder::setVolume(uint8_t volume)
{
	m_audioDecoder.setVolume(volume);
}

void Decoder::seek(uint64_t pos)
{
	if (-1 == m_seekPos)
	{
		m_seekPos = pos;
		m_DecodeCtrl.m_clock = pos;
	}
}

E_DecoderRetCode Decoder::_open(const char *pszFile)
{
	m_pFormatCtx = avformat_alloc_context();
	if (NULL == m_pFormatCtx)
	{
		return E_DecoderRetCode::DRC_Fail;
	}
	
	int iRet = avformat_open_input(&m_pFormatCtx, pszFile, NULL, NULL);
	if (0 != iRet)
	{
		this->_freeFormatCtx();
		return E_DecoderRetCode::DRC_OpenFail;
	}

	if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0)
	{
		this->_freeFormatCtx();
		return E_DecoderRetCode::DRC_NoAudioStream;
	}

	if (m_pFormatCtx->duration < 0)
	{
		this->_freeFormatCtx();
		return E_DecoderRetCode::DRC_InvalidAudioStream;
	}

	/* find video & audio stream index */
	for (unsigned int i = 0; i < m_pFormatCtx->nb_streams; i++)
	{
		if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioIndex = i;

			m_duration = m_pFormatCtx->duration;
			
			return E_DecoderRetCode::DRC_Success;
		}
	}

	this->_freeFormatCtx();

	return E_DecoderRetCode::DRC_NoAudioStream;
}

int64_t Decoder::check(const char *pszFile)
{
	E_DecoderRetCode eRetCode = _open(pszFile);
	if (E_DecoderRetCode::DRC_Success != eRetCode)
	{
		return -1;
	}

	this->_freeFormatCtx();

	return m_duration;
}

int64_t Decoder::open(const char *pszFile, const CB_DecodeFinished& cbPlayFinished)
{
	if (E_DecoderRetCode::DRC_Success != _open(pszFile))
	{
		return -1;
	}

	if (0 != m_audioDecoder.open(*m_pFormatCtx->streams[m_audioIndex], cbPlayFinished))
	{
		this->_clearData();

		return -1;
	}

	return m_duration;
}

#include <thread>

void Decoder::start()
{
	m_decodeState = E_DecodeState::DS_DECODING;

	m_DecodeCtrl.m_isReadFinished = false;
	m_DecodeCtrl.m_isStoping = false;
	m_DecodeCtrl.m_clock = 0;

	m_audioDecoder.start(m_DecodeCtrl);

	AVPacket packet;        // packet use in decoding
	while (!m_DecodeCtrl.m_isStoping)
	{
		/* do not read next frame & delay to release cpu utilization */
		if (E_DecodeState::DS_PAUSE == m_decodeState)
		{
			__sleep(10);

			continue;
		}

		/* this seek just use in playing music, while read finished
		 * & have out of loop, then jump back to seek position
		 */
	seek:
		if (m_seekPos>=0)
		{
			int64_t seekPos = av_rescale_q(m_seekPos, av_get_time_base_q(), m_pFormatCtx->streams[m_audioIndex]->time_base);

			if (av_seek_frame(m_pFormatCtx, m_audioIndex, seekPos, AVSEEK_FLAG_BACKWARD) < 0)
			{
				//QDebug() << "Seek failed.";
				if (m_DecodeCtrl.m_isReadFinished)
				{
					break;
				}
				else
				{
					continue;
				}
			}

			m_DecodeCtrl.m_isReadFinished = false;

			m_DecodeCtrl.m_isSeek = true;
			m_DecodeCtrl.m_packetQueue.empty();

			m_seekPos = -1;
		}

		/* judge haven't reall all frame */
		if (av_read_frame(m_pFormatCtx, &packet) < 0) {
			//QDebug() << "Read file completed.";
			m_DecodeCtrl.m_isReadFinished = true;
			
			__sleep(10);
			
			break;
		}

		if (packet.stream_index == m_audioIndex) {
			m_DecodeCtrl.m_packetQueue.enqueue(&packet); // audio stream
		}
		else
		{
			av_packet_unref(&packet);
		}
	}

	while (!m_DecodeCtrl.m_isStoping)
	{
		/* just use at audio playing */
		if (m_seekPos>=0)
		{
			goto seek;
		}

		__sleep(10);
	}

	/* close audio device */
	m_audioDecoder.close();

	m_DecodeCtrl.m_isSeek = false;
	m_DecodeCtrl.m_packetQueue.empty();
	//m_DecodeCtrl.m_clock = 0;

	_clearData();

	m_decodeState = E_DecodeState::DS_STOP;
}

void Decoder::pause()
{
	if (m_DecodeCtrl.m_isStoping)
	{
		return;
	}

	if (E_DecodeState::DS_DECODING == m_decodeState)
	{
		m_decodeState = E_DecodeState::DS_PAUSE;

		//av_read_pause(m_pFormatCtx); // ÔÝÍ£ÍøÂçÁ÷

		m_audioDecoder.pause(true);
	}
	else if (E_DecodeState::DS_PAUSE == m_decodeState)
	{
		//av_read_play(m_pFormatCtx); // »Ö¸´ÍøÂçÁ÷
		m_audioDecoder.pause(false);

		m_decodeState = E_DecodeState::DS_DECODING;
	}
}

void Decoder::stop()
{
	m_DecodeCtrl.m_isStoping = true;

	//while (!m_DecodeCtrl.m_isReadFinished)
	//{
	//	SDL_Delay(10);
	//}

	m_audioDecoder.pause(true);
}

void Decoder::_freeFormatCtx()
{
	if (NULL != m_pFormatCtx)
	{
		avformat_close_input(&m_pFormatCtx);
		avformat_free_context(m_pFormatCtx);
		m_pFormatCtx = NULL;
	}
}

void Decoder::_clearData()
{
	_freeFormatCtx();

	m_audioIndex = -1;

	m_duration = 0;

	m_seekPos = -1;
}
