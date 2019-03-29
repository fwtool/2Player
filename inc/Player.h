#pragma once

#pragma warning(disable: 4251)

#include <util/util.h>

using fn_voidvoid = function<void()>;

#ifdef __PlaySDKPrj
#define __PlaySDKExt __declspec(dllexport)
#else
#define __PlaySDKExt __declspec(dllimport)
#endif

using CB_PlayFinish = function<void()>;

enum class E_PlayStatus
{
	PS_Stop
	, PS_Playing
	, PS_Paused
};

class __PlaySDKExt CPlayer
{
public:
	CPlayer(){}

	static int CheckMedia(const wstring& strFile, bool bLock=true);

	static void PlayAlarm(const wstring& strFile, const fn_voidvoid& cbBlock);

private:
	CB_PlayFinish m_cbFinish;

	wstring m_strFile;
	
	int m_iDuration = 0;

	E_PlayStatus m_eStatus = E_PlayStatus::PS_Stop;

	thread m_thr;

private:
	void _play(const wstring& strFile, UINT uStartPos = 0);
	
public:
	bool Init(const CB_PlayFinish& cbFinish, UINT uVolume);

	E_PlayStatus GetPlayStatus() const
	{
		return m_eStatus;
	}

	wstring GetPlayingFile() const
	{
		return m_strFile;
	}

	int GetDuration() const
	{
		return m_iDuration;
	}

	UINT getCurrentPos() const;

	int Play(const wstring& strFile, UINT uStartPos = 0);

	//void Resume();

	void SetVolume(UINT uVolume);
	UINT GetVolume() const;
	
	void Seek(UINT uPos);

	E_PlayStatus Pause();

	UINT Stop();
};
