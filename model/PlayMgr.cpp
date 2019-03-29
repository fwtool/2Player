
#include <model.h>

CPlayMgr::CPlayMgr(CPlaylistMgr& PlaylistMgr, CDataMgr& DataMgr, IModelObserver& ModelObserver)
	: m_PlaylistMgr(PlaylistMgr)
	, m_Playlist(PlaylistMgr.GetPlayinglist())
	, m_DataMgr(DataMgr)
	, m_ModelObserver(ModelObserver)
{
}

E_PlayStatus CPlayMgr::GetPlayStatus()
{
	return m_Player.GetPlayStatus();
}

bool CPlayMgr::init(const CB_PlayFinish& cbFinish)
{
	auto& option = m_DataMgr.getOption();
	m_uPlayingItem = option.uPlayingItem;
	
	__EnsureReturn(m_Player.Init(cbFinish, (UINT)option.PlaySpiritOption.iVolume), false);

	_refresh();

	return true;
}

bool CPlayMgr::_clear()
{
	if (m_Playlist.m_lstPlayItems)
	{
		__EnsureReturn(m_PlaylistMgr.RemoveAllPlayItems(m_Playlist), false);
	}

	_refresh();

	return true;
}

bool CPlayMgr::clear()
{
	__EnsureReturn(_clear(), false);
	
	m_ModelObserver.refreshPlayingList();

	return true;
}

bool CPlayMgr::insert(const TD_IMediaList& lstMedias, bool bPlay, int iPos)
{
	__EnsureReturn(lstMedias, false);

	__EnsureReturn(m_PlaylistMgr.AddPlayItems(lstMedias, m_Playlist, iPos), false);

	if (iPos >= 0)
	{
		if ((int)m_uPlayingItem >= iPos)
		{
			m_uPlayingItem += lstMedias.size();
		}
	}
	
	this->_refresh();

	m_ModelObserver.refreshPlayingList();

	if (bPlay)
	{
		if (iPos >= 0)
		{
			(void)this->play((UINT)iPos);
		}
		else
		{
			UINT uIndex = m_Playlist.m_lstPlayItems.size() - lstMedias.size();
			(void)this->play(uIndex);
		}
	}

	return true;
}

bool CPlayMgr::assign(const TD_IMediaList& lstMedias)
{
	__EnsureReturn(this->_clear(), false);

	return this->insert(lstMedias, true);
}

int CPlayMgr::move(const TD_PlayItemList& lstPlayItems, UINT uPos)
{
	UINT uNewPos = 0;

	bool bPlayingItemSelected = false;
    m_Playlist.m_lstPlayItems.get(m_uPlayingItem, [&](CPlayItem& PlayingItem) {
		bPlayingItemSelected = lstPlayItems.includes(PlayingItem);
	});
	
	TD_PlayItemList lstUnselectedPlayItems;

	UINT uLowSelectedCount = 0;

	UINT uSelectCount = 0;
	UINT uItem = 0;
	for (auto& PlayItem : m_Playlist.m_lstPlayItems)
	{
		if (lstPlayItems.includes(&PlayItem))
		{
			if (!bPlayingItemSelected)
			{
				if (uItem < m_uPlayingItem)
				{
					uLowSelectedCount++;
				}
			}
			else
			{
				if (uItem == m_uPlayingItem)
				{
					m_uPlayingItem = uSelectCount;
				}
			}

			uSelectCount++;
		}
		else if (uItem >= uPos)
		{
			lstUnselectedPlayItems.add(PlayItem);
		}
		else
		{
			uNewPos++;
		}

		uItem++;
	}

	TD_MediaList lstSetback(lstPlayItems);
	lstSetback.add(lstUnselectedPlayItems);

	__EnsureReturn(m_PlaylistMgr.SetBackPlayItems(m_Playlist, lstSetback), -1);

	if (!bPlayingItemSelected)
	{
		m_uPlayingItem -= uLowSelectedCount;

		if (uNewPos <= m_uPlayingItem)
		{
			m_uPlayingItem += uSelectCount;
		}
	}
	else
	{
		m_uPlayingItem += uNewPos;
	}

	m_ModelObserver.refreshPlayingList();

	return uNewPos;
}

bool CPlayMgr::remove(const TD_PlayItemList& lstPlayItems)
{
	UINT uIndex = 0;
	UINT uLowCount = 0;
	bool bFlag = false;
    m_Playlist.m_lstPlayItems(0, m_uPlayingItem, [&](CPlayItem& PlayItem, size_t pos) {
		if (lstPlayItems.includes(&PlayItem))
		{
			if (pos == m_uPlayingItem)
			{
				bFlag = true;
			}
			else
			{
				uLowCount++;
			}
		}
	});

	m_uPlayingItem -= uLowCount;
	
	__EnsureReturn(m_PlaylistMgr.RemovePlayItems(lstPlayItems), false);
	
	_refresh();

	m_ModelObserver.refreshPlayingList();
	
	if (bFlag && E_PlayStatus::PS_Playing == GetPlayStatus())
	{
		(void)play(m_uPlayingItem);
	}

	return true;
}

bool CPlayMgr::checkPlayedID(UINT uID)
{
	return util::ContainerFind(m_setPlayedIDs, uID);
}

int CPlayMgr::getRandomPlayItem()
{
    ArrList<CPlayItem>& lstPlayItems = m_Playlist.m_lstPlayItems;

	vector<UINT> vctUnPlayedIDs;
    lstPlayItems([&](CPlayItem& PlayItem) {
		if (m_setPlayedIDs.find(PlayItem.m_uID) == m_setPlayedIDs.end())
		{
			vctUnPlayedIDs.push_back(PlayItem.m_uID);
		}
	});

	if (vctUnPlayedIDs.empty())
	{
		m_setPlayedIDs.clear();

        if (lstPlayItems.any([](const CPlayItem& PlayItem) {
			return PlayItem.GetDuration()>0;
		}))
		{
			return rand() % lstPlayItems.size();
		}
		else
		{
			return -1;
		}
	}
	else
	{
		UINT uID = vctUnPlayedIDs[rand() % vctUnPlayedIDs.size()];
        return lstPlayItems.find([&](const CPlayItem& PlayItem) {
			return PlayItem.m_uID == uID;
		});
	}
}

void CPlayMgr::_refresh()
{
	if (m_uPlayingItem >= m_Playlist.m_lstPlayItems.size())
	{
		m_uPlayingItem = 0;
	}

	if (!m_Playlist.m_lstPlayItems)
	{
		m_setPlayedIDs.clear();

		m_Player.Stop();
	}
	else
	{
		for (set<UINT>::iterator itPlayedID = m_setPlayedIDs.begin()
			; itPlayedID != m_setPlayedIDs.end(); )
		{
            if (0 > m_Playlist.m_lstPlayItems.find([&](const CPlayItem& PlayItem) {
				return PlayItem.m_uID == *itPlayedID;
			}))
			{
				itPlayedID = m_setPlayedIDs.erase(itPlayedID);
			}
			else
			{
				itPlayedID++;
			}
		}
	}
}

bool CPlayMgr::play(int iItem)
{
	UINT uItem = 0;
	if (-1 == iItem)
	{
		uItem = m_uPlayingItem;
	}
	else if (iItem < (int)m_Playlist.m_lstPlayItems.size())
	{
		uItem = iItem;
	}

	m_Playlist.m_lstPlayItems.get(uItem, [&](CPlayItem& PlayItem) {
		UINT uPrevPlayingItem = m_uPlayingItem;

		m_uPlayingItem = uItem;
		m_setPlayedIDs.insert(PlayItem.m_uID);

		int iDuration = m_Player.Play(PlayItem.GetAbsPath());
		if (iDuration < 0)
		{
			iDuration = 0;
		}
		PlayItem.SetDuration((UINT)iDuration);

		m_ModelObserver.onPlay(m_uPlayingItem, uPrevPlayingItem);
	});
	
	return false;
}

void CPlayMgr::_playNext(int nFlag)
{
	UINT uPlayItemCount = m_Playlist.m_lstPlayItems.size();
	__Ensure(0 < uPlayItemCount);

	int nCurrentItem = 0;

	if (0 == nFlag)
	{
		nCurrentItem = getRandomPlayItem();
		if (nCurrentItem < 0)
		{
			return;
		}
	}
	else
	{
		nCurrentItem = m_uPlayingItem + nFlag;

		if (nCurrentItem >= (int)uPlayItemCount)
		{
			nCurrentItem = 0;
		}
		else if (nCurrentItem < 0)
		{
			nCurrentItem = uPlayItemCount - 1;
		}
	}
	
	(void)this->play(nCurrentItem);
}

void CPlayMgr::playNext()
{
	if (m_DataMgr.getOption().bRandomPlay)
	{
		this->_playNext(0);
	}
	else
	{
		this->_playNext(1);
	}
}

void CPlayMgr::playLast()
{
	this->_playNext(-1);
}

bool CPlayMgr::_pause(const wstring& strPath, const function<void(wstring&)>& cb)
{
	wstring strPlayingFile = m_Player.GetPlayingFile();
	if (!strPlayingFile.empty())
	{
		if (util::StrMatchIgnoreCase(strPlayingFile, strPath)
			|| fsutil::CheckSubPath(strPath, strPlayingFile))
		{
			E_PlayStatus ePlayerStatus = GetPlayStatus();
			UINT uPos = m_Player.Stop();

			if (cb)
			{
				cb(strPlayingFile);
				if (E_PlayStatus::PS_Playing == ePlayerStatus)
				{
					if (!strPlayingFile.empty())
					{
						if (0 != uPos)
						{
							uPos += 30000;
						}
						(void)m_Player.Play(strPlayingFile, uPos);
					}
				}
			}
			
			return true;
		}
	}

	if (cb)
	{
		(void)cb(strPlayingFile);
	}

	return false;
}

void CPlayMgr::renameFile(const wstring& strPath, const function<wstring(const wstring&)>& cb)
{
    (void)_pause(strPath, [&](wstring& strPlayingFile) {
		strPlayingFile = cb(strPlayingFile);
	});
}

bool CPlayMgr::removeFile(const wstring& strPath, const function<bool()>& cb)
{
    return _pause(strPath, [&](wstring& strPlayingFile) {
		if (cb())
		{
			strPlayingFile.clear();
		}
	});
}

void CPlayMgr::moveFile(const wstring& strPath, const wstring& strNewPath, const function<bool()>& cb)
{
    _pause(strNewPath, [&](wstring&) {
        _pause(strPath, [&](wstring& strPlayingFile) {
			if (cb())
			{
				strPlayingFile = strNewPath;
            }
        });
	});
}

bool CPlayMgr::demandMedia(const TD_MediaList& lstMedias, UINT uDemandCount)
{
	__EnsureReturn(lstMedias, false);

	if (lstMedias.size() <= uDemandCount)
	{
		return this->assign(TD_IMediaList(lstMedias));
	}

	TD_IMediaList lstDemand;
	UINT uIndex = 0;
	UINT uFlag = 0;
	lstMedias([&](CMedia& media, UINT uIdx) {
		UINT uLeftCount = lstMedias.size() - uIndex;
		if (rand() % uLeftCount < uDemandCount)
		{
			if (media.CheckFileSize() > 0)
			{
				lstDemand.add(media);
				uDemandCount--;
			}
			else
			{
				uFlag++;
				if (uFlag >= uDemandCount)
				{
					uDemandCount--;
					uFlag = 0;
				}
			}

			if (0 == uDemandCount)
			{
				return false;
			}
		}

		uIndex++;

		return true;
	});

	return this->assign(lstDemand);
}

bool CPlayMgr::demandMedia(CMediaSet& rootMediaSet, E_MediaSetType nType)
{
	TD_MediaSetList lstSubSets;
	rootMediaSet.GetAllMediaSets(nType, lstSubSets);

	TD_MediaList lstMedias;
	lstSubSets([&](CMediaSet& SubSet) {
		SubSet.GetMedias(lstMedias);
	});

	return demandMedia(lstMedias);
}

bool CPlayMgr::demandMediaSet(CMediaSet& rootMediaSet, E_MediaSetType nType)
{
	TD_MediaSetList vctMediaSets;
	rootMediaSet.GetAllMediaSets(nType, vctMediaSets);

	while (vctMediaSets)
	{
		int nIndex = rand() % vctMediaSets.size();
		
		TD_MediaList lstMedias;
		vctMediaSets.get(nIndex, [&](CMediaSet& MediaSet) {
			MediaSet.GetAllMedias(lstMedias);
		});
		
		bool bFlag = false;
		lstMedias.getFront([&](CMedia& media) {
			if (media.CheckFileSize() > 0)
			{
				bFlag = true;
			}
		});

		if (!bFlag)
		{
			vctMediaSets.del_pos(nIndex);
			continue;
		}
		
		return this->assign(TD_IMediaList(lstMedias));
	}

	return false;
}
