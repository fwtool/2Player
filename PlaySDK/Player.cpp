
#include "Player.h"

#include "decoder.h"

static Decoder g_Decoder;

int CPlayer::CheckMedia(const wstring& strFile, bool bLock)
{
	string t_strFile = util::WStrToStr(strFile, CP_UTF8);

	int64_t iDuration = 0;
	Decoder Decoder;
	if (bLock)
	{
		static NS_mtutil::CCSLock s_csLock;
		s_csLock.lock();

		iDuration = Decoder.check(t_strFile.c_str());

		s_csLock.unlock();
	}
	else
	{
		iDuration = Decoder.check(t_strFile.c_str());
	}

	if (iDuration < 0)
	{
		if (fsutil_win::ExistsFile(strFile))
		{
			return 0;
		}
	}
	else if(iDuration > 0)
	{
		iDuration /= __1e6;
	}
	
	return (int)iDuration;
}

void CPlayer::PlayAlarm(const wstring& strFile, const fn_voidvoid& cbBlock)
{
	CPlayer Player;
	if (0 < Player.Play(strFile))
	{
		cbBlock();
		Player.Stop();
	}
}

UINT CPlayer::getCurrentPos() const
{
	return (UINT)g_Decoder.getClock()/ __1e6;
}

bool CPlayer::Init(const CB_PlayFinish& cbFinish, UINT uVolume)
{
	if (!CSDLDev::init())
	{
		return false;
	}

	m_cbFinish = cbFinish;

	SetVolume(uVolume);

	return true;
}

int CPlayer::Play(const wstring& strFile, UINT uStartPos)
{
	m_strFile = strFile;

	Stop();

	_play(strFile, uStartPos);

	return m_iDuration;
}

void CPlayer::_play(const wstring& strFile, UINT uStartPos)
{
	int64_t iDuration = g_Decoder.open(util::WStrToStr(strFile, CP_UTF8).c_str(), [&]() {
		if (m_cbFinish)
		{
			m_cbFinish();
		}
	});
	if (iDuration > 0)
	{
		m_thr = thread([&]() {
			g_Decoder.start();
		});

		m_eStatus = E_PlayStatus::PS_Playing;

		m_iDuration = int(iDuration/ __1e6);

		if (0 != uStartPos)
		{
			g_Decoder.seek(uStartPos);
		}
	}
	else
	{
		m_iDuration = (int)iDuration;
		this->Stop();
	}
}

//void CPlayer::Resume()
//{
//	if (!m_strFile.empty())
//	{
//		_play(m_strFile);
//	}
//}

void CPlayer::SetVolume(UINT uVolume)
{
	g_Decoder.setVolume(uVolume*255/100);
}

UINT CPlayer::GetVolume() const
{
	return 100* g_Decoder.getVolume()/255;
}

void CPlayer::Seek(UINT uPos)
{
	g_Decoder.seek(uPos*__1e6);
}

E_PlayStatus CPlayer::Pause()
{
	if (E_PlayStatus::PS_Playing == m_eStatus)
	{
		m_eStatus = E_PlayStatus::PS_Paused;
		g_Decoder.pause();
	}
	else if (E_PlayStatus::PS_Paused == m_eStatus)
	{
		m_eStatus = E_PlayStatus::PS_Playing;
		g_Decoder.pause();
	}

	return m_eStatus;
}

UINT CPlayer::Stop()
{
	if (!m_thr.joinable())
	{
		return 0;
	}

	uint64_t pos = g_Decoder.getClock();

	g_Decoder.stop();

	m_thr.join();

	m_eStatus = E_PlayStatus::PS_Stop;

	return (UINT)pos;
}
