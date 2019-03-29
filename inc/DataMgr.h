#pragma once

struct tagPlaySpiritOption
{
	int iVolume = 50;

	int iPosX = 500;
	int iPosY = 500;

	wstring strSkinName;
};

struct tagAlarmOption
{
	tagAlarmOption()
	{
		util::getCurrentTime(nHour, nMinute);
	}

	int nHour = 0;
	int nMinute = 0;

	vector<wstring> vctAlarmmedia;
};

enum E_TimerOperate
{
	TO_Null = 0
	, TO_StopPlay
	, TO_ShutdownWindow
};

struct tagTimerOperateOpt
{
	tagTimerOperateOpt()
	{
		eTimerOperate = TO_Null;

		util::getCurrentTime(nHour, nMinute);
	}

	E_TimerOperate eTimerOperate;

	int nHour = 0;
	int nMinute = 0;
};

struct tagOption
{
	wstring strRootDir;
	vector<wstring> vecAttachDir;

	UINT uPlayingItem = 0;

	bool bHideMenuBar = false;
	
	bool bFullScreen = false;

	bool bRandomPlay = false;

	tagPlaySpiritOption PlaySpiritOption;

	tagAlarmOption AlarmOption;
	
	tagTimerOperateOpt TimerOption;
};

class __ModelExt CDataMgr
{
	friend class CModel;

public:
	CDataMgr(CDao& dao)
		: m_dao(dao)
	{
	}

private:
	CDao& m_dao;

	tagOption m_Option;
	
public:
	tagOption& getOption()
	{
		return m_Option;
	}
	
	tagPlaySpiritOption& getPlaySpiritOption()
	{
		return m_Option.PlaySpiritOption;
	}

	tagAlarmOption& getAlarmOption()
	{
		return m_Option.AlarmOption;
	}

	tagTimerOperateOpt& getTimerOption()
	{
		return m_Option.TimerOption;
	}

	bool init();
	
	bool clearData();

	bool updateMediaSetName(E_MediaSetType eType, UINT uID, const wstring& strName);
	
	bool initAlarmmedia();
	
	bool addAlarmmedia(const TD_IMediaList& lstMedias);
	
	bool removeAlarmmedia(UINT uIndex);

	bool clearAlarmmedia();

	bool queryPlayRecordMaxTime(time_t& time);
	bool queryPlayRecord(time_t time, vector<pair<wstring, int>>& vecPlayRecord);
	bool clearPlayRecord();

	wstring checkAlarm();

	E_TimerOperate checkTimerOperate();
	
	bool saveOption();
};
