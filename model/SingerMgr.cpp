
#include <model.h>

#define __BASE_NAME_SINGERGROUP L"组"

#define __BASE_NAME_SINGER L"歌手"

#define __BASE_NAME_ALBUM L"专辑"

BOOL CSingerMgr::Init()
{
	auto fnAddSinger = [&](SArray<tagSinger>& arrSinger, CSingerGroup *pSingerGroup = NULL)
	{
		auto fnAddAlbum = [&](CSinger& Singer, SArray<tagAlbum>& arrAlbum) {
			for (auto& Album : arrAlbum)
			{
				Singer.m_lstAlbums.push_back(CAlbum(Album.nID, Album.strName, &Singer));
				list<CAlbumItem>& lstAlbumItems = Singer.m_lstAlbums.back().m_lstAlbumItems;

				for (auto& AlbumItem : Album.arrAlbumItem)
				{
					lstAlbumItems.push_back(CAlbumItem(AlbumItem.nID, AlbumItem.strPath, (time_t)AlbumItem.time, Singer.m_lstAlbums.back()));
				}
			}
		};

		for (auto& Singer : arrSinger)
		{
			CSinger *pSinger = NULL;
			if (NULL != pSingerGroup)
			{
				pSingerGroup->m_lstSingers.push_back(
					CSinger(*pSingerGroup, Singer.nID, Singer.strName, Singer.strPath, Singer.nPos));
				fnAddAlbum(pSingerGroup->m_lstSingers.back(), Singer.arrAlbum);
			}
			else
			{
				m_lstRootSingers.push_back(CSinger(*this, Singer.nID, Singer.strName, Singer.strPath, Singer.nPos));
				fnAddAlbum(m_lstRootSingers.back(), Singer.arrAlbum);
			}
		}
	};

	m_lstGroups.clear();
	m_lstRootSingers.clear();

	SArray<tagSingerGroup> arrSingerGroup;
	__EnsureReturn(m_dao.querySinger(arrSingerGroup), FALSE);
	
	for (auto& SingerGroup : arrSingerGroup)
	{
		if (__ID_GROUP_ROOT != SingerGroup.nID)
		{
			m_lstGroups.push_back(CSingerGroup(*this, SingerGroup.nID, SingerGroup.strName));
			fnAddSinger(SingerGroup.arrSinger, &m_lstGroups.back());
		}
		else
		{
			fnAddSinger(SingerGroup.arrSinger);
		}
	}

	return TRUE;
}

void CSingerMgr::GetAllSinger(TD_MediaSetList& lstSinger)
{
	this->GetAllMediaSets(E_MediaSetType::MST_Singer, lstSinger);
}

void CSingerMgr::GetSinger(const wstring& strDir, map<wstring, pair<UINT, wstring>>& mapSingerInfo)
{
	TD_MediaSetList lstSinger;
	GetAllSinger(lstSinger);

	wstring strSingerDir;
	lstSinger([&](CMediaSet& Singer){
		strSingerDir = Singer.GetBaseDir();
		if (strDir == fsutil::GetParentDir(strSingerDir))
		{
			mapSingerInfo[fsutil::GetFileName(strSingerDir)] = { Singer.m_uID, Singer.m_strName };
		}
	});
}

CSinger* CSingerMgr::GetSinger(const wstring& strMediaPath)
{
	TD_MediaSetList lstSinger;
	GetAllSinger(lstSinger);

	CSinger *pSinger = NULL;
	lstSinger([&](CMediaSet& Singer) {
		if (fsutil::CheckSubPath(Singer.GetBaseDir(), strMediaPath))
		{
			pSinger = (CSinger*)&Singer;
			return false;
		}
		return true;
	});

	return pSinger;
}

void CSingerMgr::GetSubSets(TD_MediaSetList& lstSubSets)
{
	lstSubSets.add(m_lstGroups);
	lstSubSets.add(m_lstRootSingers);
}

CSingerGroup *CSingerMgr::AddGroup()
{
	wstring strName = m_dao.getNewSingerGroupName(__BASE_NAME_SINGERGROUP);
	__EnsureReturn(!strName.empty(), NULL);

	int iID = m_dao.addSingerGroup(strName);
	__EnsureReturn(0 < iID, NULL);

	m_lstGroups.push_back(CSingerGroup(*this, iID, strName));

	return &m_lstGroups.back();
}

BOOL CSingerMgr::RemoveGroup(UINT uSingerGroupID)
{
	__EnsureReturn(m_dao.deleteSingerGroup(uSingerGroupID), FALSE);

	__EnsureReturn(this->Init(), FALSE);

	return TRUE;
}


CSinger *CSingerMgr::AddSinger(CMediaRes& SrcPath, CSingerGroup *pGroup, bool bInitAlbum)
{
	list<pair<wstring, TD_IMediaList>> lstAlbums;
	if (bInitAlbum)
	{
		TD_PathList lstDirs(SrcPath);
		SrcPath.GetSubDir(lstDirs);

		lstDirs([&](CPath& DirObject){
			CMediaRes& Dir = (CMediaRes&)DirObject;

			TD_PathList lstFiles;
			Dir.GetSubFile(lstFiles);

			if (lstFiles)
			{
				TD_MediaResList lstMediaRes(lstFiles);
				TD_IMediaList lstMedias(lstMediaRes);
				lstAlbums.push_back(std::make_pair(Dir.GetName(), lstMedias));
			}
		});
	}

	wstring strSingerName = SrcPath.GetName();
	wstring strPath = SrcPath.GetPath();

	CDaoTransGuard transGuard(m_dao);

	int nGroupID = __ID_GROUP_ROOT;
	CMediaSet *pParent = NULL;
	list<CSinger> *plstSingers = NULL;
	if (NULL != pGroup)
	{
		nGroupID = pGroup->m_uID;
		pParent = pGroup;
		plstSingers = &pGroup->m_lstSingers;
	}
	else
	{
		pParent = this;
		plstSingers = &m_lstRootSingers;
	}

	int nPos = (int)plstSingers->size();

	int iMaxID = m_dao.addSinger(nGroupID, strSingerName, strPath, nPos);
	__EnsureReturn(0 < iMaxID, NULL);

	plstSingers->push_back(CSinger(*pParent, iMaxID, strSingerName, strPath, nPos));
	CSinger *pSinger = &plstSingers->back();

	for (auto& pr : lstAlbums)
	{
		(void)this->AddAlbum(*pSinger, pr.first, &pr.second);
	}
	
	return pSinger;
}

BOOL CSingerMgr::RemoveSinger(UINT uSingerID)
{
	__EnsureReturn(m_dao.deleteSinger(uSingerID), FALSE);

	__EnsureReturn(this->Init(), FALSE);

	return TRUE;
}

BOOL CSingerMgr::UpdateSingerPos(UINT uSingerID, int nPos, int nDstGroupID)
{
	__EnsureReturn(m_dao.updateSingerPos(uSingerID, nPos, nDstGroupID), FALSE);

	__EnsureReturn(this->Init(), FALSE);

	return TRUE;
}

CAlbum *CSingerMgr::AddAlbum(CSinger& Singer, wstring strName, const TD_IMediaList *plstAlbumItem)
{
	if (strName.empty())
	{
		strName = m_dao.getNewAlbumName(Singer.m_uID, __BASE_NAME_ALBUM);
		__EnsureReturn(!strName.empty(), NULL);
	}

	int iID = m_dao.addAlbum(strName, Singer.m_uID, Singer.m_lstAlbums.size());
	__EnsureReturn(0 < iID, NULL);

	Singer.m_lstAlbums.push_back(CAlbum(iID, strName, &Singer));
	CAlbum *pAlbum = &Singer.m_lstAlbums.back();
		
	if (plstAlbumItem)
	{
		(void)AddAlbumItems(*plstAlbumItem, *pAlbum);
	}
	
	return pAlbum;
}

BOOL CSingerMgr::RemoveAlbum(UINT uID)
{
	CMediaSet *pAlbum = this->GetMediaSet(uID, E_MediaSetType::MST_Album);
	__EnsureReturn(pAlbum, FALSE);

	CSinger *pSinger = (CSinger*)pAlbum->m_pParent;
	__EnsureReturn(pSinger, FALSE);

	for (auto itr = pSinger->m_lstAlbums.begin(); itr != pSinger->m_lstAlbums.end(); ++itr)
	{
		if (itr->m_uID == uID)
		{
			__EnsureReturn(m_dao.deleteAlbum(uID, pSinger->m_uID), FALSE);

			pSinger->m_lstAlbums.erase(itr);

			return TRUE;
		}
	}

	return FALSE;
}

CAlbum *CSingerMgr::RepositAlbum(CAlbum& Album, int nNewPos)
{
	CSinger& Singer = Album.GetSinger();

	int nOldPos = 0;
	list<CAlbum>::iterator itAlbum = Singer.m_lstAlbums.begin();
	for (; ; ++itAlbum, ++nOldPos)
	{
		__EnsureReturn(Singer.m_lstAlbums.end() != itAlbum, NULL);

		if (&*itAlbum == &Album)
		{
			break;
		}
	}

	__EnsureReturn(nNewPos != nOldPos, NULL);

	__EnsureReturn(m_dao.updateAlbumPos(Album.m_uID, nOldPos, nNewPos, Singer.m_uID), NULL);


	CAlbum tempAlbum(Album);

	Singer.m_lstAlbums.erase(itAlbum);

	itAlbum = Singer.m_lstAlbums.begin();
	for (int nIndex = 0; nIndex < nNewPos; ++nIndex, itAlbum++)
	{
		if (itAlbum == Singer.m_lstAlbums.end())
		{
			break;
		}		
	}

	itAlbum = Singer.m_lstAlbums.insert(itAlbum, CAlbum(tempAlbum.m_uID, tempAlbum.m_strName, (CSinger*)tempAlbum.m_pParent));
	(*itAlbum).AddAlbumItems(tempAlbum.m_lstAlbumItems);

	return &*itAlbum;
}

BOOL CSingerMgr::AddAlbumItems(const list<wstring>& lstMediaPaths, CAlbum& Album, int nPos)
{
	TD_MediaList lstAlbumItems;
	if (-1 != nPos)
	{
		int nIndex = 0;
		for (list<CAlbumItem>::iterator itAlbumItem = Album.m_lstAlbumItems.begin()
			; itAlbumItem != Album.m_lstAlbumItems.end(); ++itAlbumItem, ++nIndex)
		{
			if (nIndex >= nPos)
			{
				lstAlbumItems.add(*itAlbumItem);
			}
		}
	}

	auto strBaseDir = Album.GetBaseDir();
	list<wstring> lstOppPaths;
	for (auto& strMediaPath : lstMediaPaths)
	{
		lstOppPaths.push_back(fsutil::GetOppPath(strMediaPath, strBaseDir));
	}

    m_dao.addAlbumItem(lstOppPaths, Album.m_uID, nPos, [&](UINT uAlbumItemID, const wstring& strOppPaths, time_t time) {
		Album.m_lstAlbumItems.push_back(CAlbumItem(uAlbumItemID, strOppPaths, time, Album));
	});
	
	if (lstAlbumItems)
	{
		return SetBackAlbumItems(lstAlbumItems);
	}

	return TRUE;
}

BOOL CSingerMgr::AddAlbumItems(const TD_IMediaList& lstMedias, CAlbum& Album, int nPos)
{
	list<wstring> lstMediaPaths;
	lstMedias([&](IMedia& Media){
		lstMediaPaths.push_back(Media.GetPath());
	});
	return AddAlbumItems(lstMediaPaths, Album, nPos);
}

BOOL CSingerMgr::RemoveAlbumItems(const TD_AlbumItemList& lstAlbumItems)
{
	list<UINT> lstAlbumItemIDs;
	lstAlbumItems([&](CAlbumItem& AlbumItem){
		list<CAlbumItem>& lstAlbumItems = AlbumItem.GetAlbum()->m_lstAlbumItems;
		for (list<CAlbumItem>::iterator itAlbumItem = lstAlbumItems.begin()
			; itAlbumItem != lstAlbumItems.end(); ++itAlbumItem)
		{
			if (&*itAlbumItem == &AlbumItem)
			{
				lstAlbumItemIDs.push_back(AlbumItem.m_uID);

				lstAlbumItems.erase(itAlbumItem);

				break;
			}
		}

		return true;
	});

	if (!lstAlbumItemIDs.empty())
	{
		__EnsureReturn(m_dao.deleteAlbumItem(lstAlbumItemIDs), FALSE);
	}

	return TRUE;
}

BOOL CSingerMgr::SetBackAlbumItems(const TD_MediaList& lstAlbumItems)
{
	BOOL bRet = false;
	lstAlbumItems.getFront([&](CMedia& media) {
		CAlbum *pAlbum = (CAlbum*)media.m_pParent;
	
		int nMaxPos = m_dao.queryMaxAlbumItemPos(pAlbum->m_uID);

		list<UINT> lstAlbumItemID;
		lstAlbumItems([&](CMedia& Media){
			lstAlbumItemID.push_back(Media.m_uID);

			pAlbum->m_lstAlbumItems.push_back((CAlbumItem&)Media);
		});
		__Ensure(m_dao.setbackAlbumItem(pAlbum->m_uID, lstAlbumItemID));

		for (list<CAlbumItem>::iterator itAlbumItem = pAlbum->m_lstAlbumItems.begin()
			; itAlbumItem != pAlbum->m_lstAlbumItems.end(); )
		{
			if (lstAlbumItems.includes(&*itAlbumItem))
			{
				itAlbumItem = pAlbum->m_lstAlbumItems.erase(itAlbumItem);
				continue;
			}

			++itAlbumItem;
		}

		bRet = TRUE;
	});

	return bRet;
}

CSinger* CSingerMgr::RepositSinger(CSinger& Singer, CMediaSet& Target, bool bDropPositionFlag)
{
	int nSingerID = Singer.m_uID;

	int nPos = 0;

	CSingerGroup *pDstGroup = NULL;

	if (E_MediaSetType::MST_Singer == Target.m_eType)
	{
		__EnsureReturn(&Target != &Singer, NULL);

		nPos = ((CSinger&)Target).m_nPos;
		if (bDropPositionFlag)
		{
			nPos++;
		}

		CSingerGroup *pSrcGroup = (CSingerGroup*)Singer.m_pParent;
		pDstGroup = (CSingerGroup*)Target.m_pParent;

		if (pSrcGroup == pDstGroup)
		{
			if (nPos > Singer.m_nPos)
			{
				nPos--;
			}
			__EnsureReturn(nPos != Singer.m_nPos, NULL);
		}

		__EnsureReturn(this->UpdateSingerPos(Singer.m_uID, nPos, pDstGroup->m_uID), NULL);
	}
	else
	{
		__EnsureReturn(Singer.m_pParent != &Target, NULL);

		pDstGroup = (CSingerGroup*)&Target;

		__EnsureReturn(this->UpdateSingerPos(Singer.m_uID, (int)pDstGroup->m_lstSingers.size(), pDstGroup->m_uID), NULL);
	}
	
	return (CSinger*)GetMediaSet(nSingerID, E_MediaSetType::MST_Singer);
}

int CSingerMgr::RepositAlbumItem(CAlbum& Album, const TD_IMediaList& lstMedias, UINT uTargetPos)
{
	int nNewPos = 0;
	
	UINT uItem = 0;
	TD_MediaList lstUnselectedAlbumItems;
	for (list<CAlbumItem>::iterator itrPlayItem = Album.m_lstAlbumItems.begin()
		; itrPlayItem != Album.m_lstAlbumItems.end(); ++itrPlayItem)
	{
		if (!lstMedias.includes(&*itrPlayItem))
		{
			if (uItem >= uTargetPos)
			{
				lstUnselectedAlbumItems.add(*itrPlayItem);
			}
			else
			{
				nNewPos++;
			}
		}

		uItem++;
	}

	TD_MediaList lstAlbumItem(lstMedias);
	lstAlbumItem.add(lstUnselectedAlbumItems);

	__EnsureReturn(this->SetBackAlbumItems(lstAlbumItem), -1);

	return nNewPos;
}
