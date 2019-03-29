
#include <model.h>

#define __BASE_NAME_PLAYLIST L"播放列表"

BOOL CPlaylistMgr::Init()
{
	SArray<tagPlaylist> arrPlaylist;
	__EnsureReturn(m_dao.queryPlaylist(arrPlaylist), FALSE);

	for (auto&& Playlist : arrPlaylist)
	{
		CPlaylist* pPlaylist = NULL;
		if (ID_PLAYLIST_NULL == Playlist.nID)
		{
			pPlaylist = &this->m_Playinglist;
		}
		else
		{
			m_lstPlaylists.push_back(CPlaylist(*this, Playlist.nID, Playlist.strName));
			pPlaylist = &m_lstPlaylists.back();
		}

		for (auto& PlayItem : Playlist.arrPlayItem)
		{
			pPlaylist->m_lstPlayItems.add(CPlayItem(PlayItem.nID, PlayItem.strPath, (time_t)PlayItem.time, *pPlaylist));
		}
	}

	return TRUE;
}

void CPlaylistMgr::GetSubSets(TD_MediaSetList& lstSubSets)
{
	lstSubSets.add(m_lstPlaylists);
}

CPlaylist *CPlaylistMgr::AddPlaylist(UINT uPos)
{
	wstring strName = m_dao.getNewPlaylistName(__BASE_NAME_PLAYLIST);
	__EnsureReturn(!strName.empty(), NULL);

	int iID = m_dao.addPlaylist(strName);
	__EnsureReturn(0 < iID, NULL);

	m_lstPlaylists.push_front(CPlaylist(*this, iID, strName));

	return RepositPlaylist(iID, uPos);
}

BOOL CPlaylistMgr::RemovePlaylist(UINT uID)
{
	for (auto itr = m_lstPlaylists.begin(); itr != m_lstPlaylists.end(); ++itr)
	{
		if (itr->m_uID == uID)
		{
			__EnsureReturn(m_dao.deletePlaylist(uID), FALSE);

			m_lstPlaylists.erase(itr);

			return TRUE;
		}
	}

	return FALSE;
}

CPlaylist *CPlaylistMgr::RepositPlaylist(UINT uID, UINT uNewPos)
{
	UINT uOldPos = 0;

	auto itr = m_lstPlaylists.begin();
	for (; ; ++itr, ++uOldPos)
	{
		__EnsureReturn(itr != m_lstPlaylists.end(), NULL);

		if (itr->m_uID == uID)
		{
			break;
		}
	}

	if (uNewPos != uOldPos)
	{
		__EnsureReturn(m_dao.updatePlaylistPos(uID, uOldPos, uNewPos), NULL);
	}

	CPlaylist tempPlaylist(*itr);

	m_lstPlaylists.erase(itr);

	itr = m_lstPlaylists.begin();
	for (UINT uIndex = 0; uIndex < uNewPos; ++uIndex)
	{
		if (itr == m_lstPlaylists.end())
		{
			break;
		}

		itr++;
	}
	itr = m_lstPlaylists.insert(itr, tempPlaylist);

	return &*itr;
}

BOOL CPlaylistMgr::AddPlayItems(const SArray<wstring>& lstOppPaths, CPlaylist& Playlist, int nPos)
{
	TD_MediaList lstPlayItems;
	if (-1 != nPos)
	{
		int nIndex = 0;
		for (list<CPlayItem>::iterator itPlayItem = Playlist.m_lstPlayItems.begin()
			; itPlayItem != Playlist.m_lstPlayItems.end(); ++itPlayItem, ++nIndex)
		{
			if (nIndex >= nPos)
			{
				lstPlayItems.add(*itPlayItem);
			}
		}
	}
	
	__EnsureReturn(m_dao.addPlayItem(lstOppPaths, Playlist.m_uID, nPos
		, [&](UINT uPlayItemID, wstring strPath, time_t time){
			Playlist.m_lstPlayItems.add(CPlayItem(uPlayItemID, strPath, time, Playlist));
	}), FALSE);
	
	if (lstPlayItems)
	{
		return SetBackPlayItems(Playlist, lstPlayItems);
	}

	return TRUE;
}

BOOL CPlaylistMgr::AddPlayItems(const TD_IMediaList& lstMedias, CPlaylist& Playlist, int nPos)
{
	SArray<wstring> lstOppPaths = lstMedias.map([](IMedia& Media){
		return Media.GetPath();
	});

	return AddPlayItems(lstOppPaths, Playlist, nPos);
}

BOOL CPlaylistMgr::RemovePlayItems(const TD_PlayItemList& arrPlayItems)
{
	list<UINT> lstIDs;

	arrPlayItems([&](CPlayItem& PlayItem){
		auto& lstPlayItems = PlayItem.GetPlaylist()->m_lstPlayItems;
		for (auto itr = lstPlayItems.begin(); itr != lstPlayItems.end(); ++itr)
		{
			if (&*itr == &PlayItem)
			{
				lstIDs.push_back(PlayItem.m_uID);

				lstPlayItems.erase(itr);

				break;
			}
		}

		return true;
	});

	if (!lstIDs.empty())
	{
		__EnsureReturn(m_dao.deletePlayItem(lstIDs), FALSE);
	}

	return TRUE;
}

BOOL CPlaylistMgr::RemoveAllPlayItems(CPlaylist& Playlist)
{
	__EnsureReturn(m_dao.deletePlayItem(Playlist.m_uID), FALSE);

	Playlist.m_lstPlayItems.clear();

	return TRUE;
}

int CPlaylistMgr::RepositPlayItem(CPlaylist& Playlist, const TD_IMediaList& lstMedias, UINT uTargetPos)
{
	int nNewPos = 0;

	UINT uItem = 0;
	TD_MediaList lstUnselectedPlayItems;
	for (list<CPlayItem>::iterator itrPlayItem = Playlist.m_lstPlayItems.begin()
		; itrPlayItem != Playlist.m_lstPlayItems.end(); ++itrPlayItem)
	{
		if (!lstMedias.includes(&*itrPlayItem))
		{
			if (uItem >= uTargetPos)
			{
				lstUnselectedPlayItems.add(*itrPlayItem);
			}
			else
			{
				nNewPos++;
			}
		}

		uItem++;
	}

	TD_MediaList lstPlayItems(lstMedias);
	lstPlayItems.add(lstUnselectedPlayItems);

	__EnsureReturn(this->SetBackPlayItems(Playlist, lstPlayItems), -1);

	return nNewPos;
}

BOOL CPlaylistMgr::SetBackPlayItems(CPlaylist& Playlist, const TD_MediaList& lstPlayItems)
{
	list<UINT> lstIDs;
	lstPlayItems([&](CMedia& Media) {
		lstIDs.push_back(Media.m_uID);
	});

	__EnsureReturn(m_dao.setbackPlayItem(Playlist.m_uID, lstIDs), FALSE);

	lstPlayItems([&](CMedia& Media) {
		Playlist.m_lstPlayItems.add((CPlayItem&)Media);
	});
	
	for (list<CPlayItem>::iterator itPlayItem = Playlist.m_lstPlayItems.begin()
		; itPlayItem != Playlist.m_lstPlayItems.end(); )
	{
		if (lstPlayItems.includes(&*itPlayItem))
		{
			itPlayItem = Playlist.m_lstPlayItems.erase(itPlayItem);
		}
		else
		{
			itPlayItem++;
		}
	}

	return TRUE;
}

