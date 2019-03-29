
#include <MediaDef.h>

CPlayItem::CPlayItem(int nID, const wstring& strPath, time_t time, CPlaylist& Playlist)
	: CMedia(&Playlist, nID, strPath, time)
{
}

wstring CPlayItem::_GetDisplayName()
{
	if (!isPlayingItem())
	{
		wchar_t lpBuff[8];
		memset(lpBuff, 0, sizeof lpBuff);
		wsprintf(lpBuff, L"%03d", ((CPlaylist*)m_pParent)->GetPlayItemPos(*this));

		return wstring(lpBuff) + __CNDotDelimiter + m_strTitle;
	}

	return m_strTitle;
}

int CPlayItem::getSingerImg() const
{
	int iRelatedSingerID = GetRelatedMediaSetID(E_MediaSetType::MST_Singer);
	if (iRelatedSingerID > 0)
	{
		__EnsureReturn(g_pRootMediaRes, -1);
		UINT uSingerImgPos = g_pRootMediaRes->m_MediaObserver.getSingerImgPos((UINT)iRelatedSingerID);
		if ((UINT)E_GlobalImage::GI_SingerDefault != uSingerImgPos)
		{
			return uSingerImgPos;
		}
	}

	return -1;
}

void CPlayItem::GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage)
{
	if (this->getCueFile())
	{
		iImage = (int)E_GlobalImage::GI_DiskFile;
	}
	else
	{
		iImage = getSingerImg();
		if (iImage < 0)
		{
			iImage = (int)E_GlobalImage::GI_PlayItem;
		}
	}

	vecText.push_back(this->_GetDisplayName().c_str());

	vecText.push_back(GetExtName().substr(1));

	vecText.push_back(GetFileSizeString());

	vecText.push_back(GetDurationString());
	
	wstring strSingerAlbum = GetRelatedMediaSetName(E_MediaSetType::MST_Singer);
	wstring strAlbum = GetRelatedMediaSetName(E_MediaSetType::MST_Album);
	if (!strAlbum.empty())
	{
		strSingerAlbum.append(__CNDotDelimiter + strAlbum);
	}
	vecText.push_back(strSingerAlbum);
	
	if (!m_strParentDir.empty())
	{
		vecText.push_back(m_strParentDir);
	}
	else
	{
		__Ensure(g_pRootMediaRes);
		vecText.push_back(g_pRootMediaRes->GetAbsPath());
	}

	vecText.push_back(m_addTime.GetText());
}

void CPlayItem::onAsyncTask()
{
    CMedia::onAsyncTask();

	__Ensure(g_pRootMediaRes);
	(void)g_pRootMediaRes->m_MediaObserver.findRelatedMedia(*this, E_MediaSetType::MST_Album);
}

bool CPlayItem::CheckRelatedMediaSetChanged(const tagMediaSetChanged& MediaSetChanged)
{
	if (E_MediaSetType::MST_Singer == MediaSetChanged.eMediaSetType)
	{
		if (GetRelatedMediaSetID(E_MediaSetType::MST_Singer) == MediaSetChanged.uMediaSetID)
		{
			return true;
		}
	}

	return false;
}

CPlaylist* CPlayItem::GetPlaylist() const
{
	return (CPlaylist*)m_pParent;
}

wstring CPlayItem::GetPlaylistName() const
{
	if (NULL != m_pParent)
	{
		return m_pParent->m_strName;
	}

	return L"";
}

CPlaylist::CPlaylist(CMediaSet& parent, int nID, const wstring& strName)
	: CMediaSet(strName, &parent, nID, E_MediaSetType::MST_Playlist)
{
}

CPlaylist::CPlaylist(const CPlaylist& other)
{
	*this = other;

	for (auto& PlayItem : m_lstPlayItems)
	{
		PlayItem.m_pParent = this;
	}
}

wstring CPlaylist::_GetDisplayName()
{
	wchar_t lpBuff[8];
	memset(lpBuff, 0, sizeof lpBuff);
	wsprintf(lpBuff, L"%02d", GetIndex());

	return wstring(lpBuff) + __CNDotDelimiter + m_strName;
}

int CPlaylist::GetPlayItemPos(CPlayItem& PlayItem)
{
	return m_lstPlayItems.indexOf(PlayItem)+1;
}

