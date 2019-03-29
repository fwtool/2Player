
#include "model.h"

bool CDataMgr::init()
{	
	(void)m_dao.GetOption(OI_RootDir, m_Option.strRootDir);

	wstring strAttachDirList;
	(void)m_dao.GetOption(OI_AttachDir, strAttachDirList);
	if (!strAttachDirList.empty())
	{
		util::SplitString(strAttachDirList, L'|', m_Option.vecAttachDir);
	}

	int nPlayingItem = 0;
	(void)m_dao.GetOption(OI_PlayingItem, nPlayingItem);
	if (0 <= nPlayingItem)
	{
		m_Option.uPlayingItem = (UINT)nPlayingItem;
	}

	(void)m_dao.GetOption(OI_IfRandomPlay, m_Option.bRandomPlay);

	(void)m_dao.GetOption(OI_PlaySpiritVolum, m_Option.PlaySpiritOption.iVolume);

	(void)m_dao.GetOption(OI_PlaySpiritLeft, m_Option.PlaySpiritOption.iPosX);
	(void)m_dao.GetOption(OI_PlaySpiritTop, m_Option.PlaySpiritOption.iPosY);

	(void)m_dao.GetOption(OI_PlaySpiritSkin, m_Option.PlaySpiritOption.strSkinName);

	(void)m_dao.GetOption(OI_AlarmHour, m_Option.AlarmOption.nHour);
	(void)m_dao.GetOption(OI_AlarmMinute, m_Option.AlarmOption.nMinute);

	__EnsureReturn(initAlarmmedia(), false);

	int nHideMenuBar = 0;
	__EnsureReturn(m_dao.GetOption(OI_HideMenuBar, nHideMenuBar), false);
	m_Option.bHideMenuBar = 1 == nHideMenuBar;

	int nFullScreen = 0;
	__EnsureReturn(m_dao.GetOption(OI_FullScreen, nFullScreen), false);
	m_Option.bFullScreen = 1 == nFullScreen;

	return true;
}

bool CDataMgr::clearData()
{
	__EnsureReturn(m_dao.clearAll(), false);

	m_Option.uPlayingItem = 0;
	m_Option.AlarmOption.vctAlarmmedia.clear();

	return true;
}

bool CDataMgr::updateMediaSetName(E_MediaSetType eType, UINT uID, const wstring& strName)
{
	switch (eType)
	{
	case E_MediaSetType::MST_Playlist:
		return m_dao.updatePlaylistName(uID, strName);

		break;
	case E_MediaSetType::MST_SingerGroup:
		return m_dao.updateSingerGroupName(uID, strName);

		break;
	case E_MediaSetType::MST_Singer:
		return m_dao.updateSingerName(uID, strName);

		break;
	case E_MediaSetType::MST_Album:
		return m_dao.updateAlbumName(uID, strName);

		break;
	default:
		return false;
	}
}

bool CDataMgr::initAlarmmedia()
{
	m_Option.AlarmOption.vctAlarmmedia.clear();
	return m_dao.queryAlarmmedia(m_Option.AlarmOption.vctAlarmmedia);
}

bool CDataMgr::addAlarmmedia(const TD_IMediaList& lstMedias)
{
	SArray<wstring> vecAlarmmedia = lstMedias.map([](IMedia& Media) {
		return Media.GetPath();
	});

	__EnsureReturn(m_dao.addAlarmmedia(vecAlarmmedia), false);

	m_Option.AlarmOption.vctAlarmmedia.insert(m_Option.AlarmOption.vctAlarmmedia.end(), vecAlarmmedia.begin(), vecAlarmmedia.end());

	return true;
}

bool CDataMgr::removeAlarmmedia(UINT uIndex)
{
	vector<wstring>& vctAlarmmedia = m_Option.AlarmOption.vctAlarmmedia;
	__EnsureReturn(uIndex < vctAlarmmedia.size(), false);

	__EnsureReturn(m_dao.deleteAlarmmedia(vctAlarmmedia[uIndex]), false);

	vctAlarmmedia.erase(vctAlarmmedia.begin() + uIndex);

	return true;
}

bool CDataMgr::clearAlarmmedia()
{
	__EnsureReturn(m_dao.clearAlarmmedia(), false);

	m_Option.AlarmOption.vctAlarmmedia.clear();

	return true;
}

bool CDataMgr::queryPlayRecordMaxTime(time_t& time)
{
	return m_dao.queryPlayRecordMaxTime(time);
}

bool CDataMgr::queryPlayRecord(time_t time, vector<pair<wstring, int>>& vecPlayRecord)
{
	return m_dao.queryPlayRecord(time, vecPlayRecord);
}

bool CDataMgr::clearPlayRecord()
{
	return m_dao.clearPlayRecord();
}

wstring CDataMgr::checkAlarm()
{
	int nHour = 0, nMinute = 0;
	util::getCurrentTime(nHour, nMinute);

	auto& AlarmOption = m_Option.AlarmOption;
	if (nHour == AlarmOption.nHour && nMinute == AlarmOption.nMinute)
	{
		auto& vctAlarmmedia = AlarmOption.vctAlarmmedia;
		if (!vctAlarmmedia.empty())
		{
			return vctAlarmmedia[rand() % vctAlarmmedia.size()];
		}
	}

	return L"";
}

E_TimerOperate CDataMgr::checkTimerOperate()
{
	int nHour = 0, nMinute = 0;
	util::getCurrentTime(nHour, nMinute);

	auto& TimerOption = m_Option.TimerOption;
	if (TO_Null != TimerOption.eTimerOperate)
	{
		if (nHour == TimerOption.nHour && nMinute == TimerOption.nMinute)
		{
			E_TimerOperate eRet = TimerOption.eTimerOperate;
			TimerOption.eTimerOperate = TO_Null;

			return eRet;
		}
	}

	return TO_Null;
}

bool CDataMgr::saveOption()
{
	wstring strAttachDirList;
	for (auto& strAttachDir : m_Option.vecAttachDir)
	{
		if (!strAttachDirList.empty())
		{
			strAttachDirList.append(L"|");
		}

		strAttachDirList.append(strAttachDir);
	}
	(void)m_dao.SetOption(OI_AttachDir, strAttachDirList);

	__EnsureReturn(m_dao.SetOption(OI_PlayingItem, m_Option.uPlayingItem), false);

	__EnsureReturn(m_dao.SetOption(OI_IfRandomPlay, m_Option.bRandomPlay), false);

	__EnsureReturn(m_dao.SetOption(OI_PlaySpiritLeft, m_Option.PlaySpiritOption.iPosX), false);
	__EnsureReturn(m_dao.SetOption(OI_PlaySpiritTop, m_Option.PlaySpiritOption.iPosY), false);

	__EnsureReturn(m_dao.SetOption(OI_PlaySpiritVolum, m_Option.PlaySpiritOption.iVolume), false);
	
	__EnsureReturn(m_dao.SetOption(OI_PlaySpiritSkin, m_Option.PlaySpiritOption.strSkinName), false);

	__EnsureReturn(m_dao.SetOption(OI_AlarmHour, m_Option.AlarmOption.nHour), false);
	__EnsureReturn(m_dao.SetOption(OI_AlarmMinute, m_Option.AlarmOption.nMinute), false);

	__EnsureReturn(m_dao.SetOption(OI_HideMenuBar, m_Option.bHideMenuBar ? 1 : 0), false);

	__EnsureReturn(m_dao.SetOption(OI_FullScreen, m_Option.bFullScreen ? 1 : 0), false);

	return true;
}
