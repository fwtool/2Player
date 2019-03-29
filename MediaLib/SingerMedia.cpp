
#include <MediaDef.h>

CAlbumItem::CAlbumItem(int nID, const wstring& strPath, time_t time, CAlbum& Album)
	: CMedia(&Album, nID, strPath, time)
{
}

void CAlbumItem::onAsyncTask()
{
    CMedia::onAsyncTask();

	__Ensure(g_pRootMediaRes);
	(void)g_pRootMediaRes->m_MediaObserver.findRelatedMedia(*this, E_MediaSetType::MST_Playlist);
}

void CAlbumItem::GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage)
{
	if (this->getCueFile())
	{
		iImage = (int)E_GlobalImage::GI_DiskFile;
	}
	else
	{
		iImage = (int)E_GlobalImage::GI_AlbumItem;
	}

	vecText.push_back(bReportView ? m_strTitle : m_strName);
	
	vecText.push_back(GetExtName().substr(1));

	vecText.push_back(GetFileSizeString());

	vecText.push_back(GetDurationString());

    vecText.push_back(CMedia::GetRelatedMediaSetName(E_MediaSetType::MST_Playlist));

	if (!m_strParentDir.empty())
	{
		vecText.push_back(L"..." + m_strParentDir);
	}
	else
	{
		vecText.push_back(GetBaseDir());
	}

	vecText.push_back(m_addTime.GetText());
}

CAlbum* CAlbumItem::GetAlbum() const
{
	return (CAlbum*)m_pParent;
}

wstring CAlbumItem::GetAlbumName() const
{
	if (NULL != m_pParent)
	{
		return m_pParent->m_strName;
	}

	return L"";
}

CSinger *CAlbumItem::GetSinger() const
{
	auto pAlbum = m_pParent;
	if (pAlbum)
	{
		return (CSinger*)pAlbum->m_pParent;
	}

	return NULL;
}

wstring CAlbumItem::GetSingerName() const
{
	CSinger *pSinger = GetSinger();
	if (NULL != pSinger)
	{
		return pSinger->m_strName;
	}

	return L"";
}

void CAlbum::AddAlbumItems(const list<CAlbumItem>& lstAlbumItems)
{
	for (list<CAlbumItem>::const_iterator itAlbumItem = lstAlbumItems.begin()
		; itAlbumItem != lstAlbumItems.end(); ++itAlbumItem)
	{
		m_lstAlbumItems.push_back(*itAlbumItem);
		((CMedia*)&m_lstAlbumItems.back())->m_pParent = this;
	}
}

void CAlbum::GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage)
{
	iImage = (int)E_GlobalImage::GI_Album;

	vecText.push_back((__Space + m_strName).c_str());
}

wstring CAlbum::GetExportPosition()
{
	wstring strPosition = m_pParent->GetExportPosition();
	strPosition += fsutil::backSlant + m_pParent->m_strName + __CNDotDelimiter + m_strName;

	return strPosition;
}

class CSinger& CAlbum::GetSinger() const
{
	return (CSinger&)*m_pParent;
}


CSinger::CSinger(CMediaSet& parent, int nID, const wstring& strName, const wstring& strDir, int nPos)
	: CMediaSet(strName, &parent, nID, E_MediaSetType::MST_Singer)
	, m_strDir(strDir)
	, m_nPos(nPos)
{
}

int CSinger::GetTreeImage()
{
	if (NULL != g_pRootMediaRes)
	{
		m_uImgPos = g_pRootMediaRes->m_MediaObserver.getSingerImgPos(this->m_uID);
	}

	return m_uImgPos;
}

bool CSinger::FindMedia(const tagFindMediaPara& FindPara, tagFindMediaResult& FindResult)
{
	if (!FindPara.strDir.empty())
	{
		if (E_FindMediaMode::FMM_RenameDir == FindPara.eFindMediaMode)
		{
			if (util::StrMatchIgnoreCase(m_strDir, FindPara.strDir)
				|| fsutil::CheckSubPath(FindPara.strDir, m_strDir))
			{
				m_strDir = FindPara.strRenameDir + fsutil::GetOppPath(m_strDir, FindPara.strDir);
				FindResult.mapSingerDirChanged[m_uID] = m_strDir;

				return true;
			}
		}

		if (util::StrMatchIgnoreCase(FindPara.strDir, m_strDir)
			|| fsutil::CheckSubPath(m_strDir, FindPara.strDir))
		{
			FindResult.pRetSinger = this;
		}
		else if (!fsutil::CheckSubPath(FindPara.strDir, m_strDir))
		{
			return true;
		}
	}

    return CMediaSet::FindMedia(FindPara, FindResult);
}


CSingerGroup::CSingerGroup(CMediaSet& parent, int nID, const wstring& strName)
	: CMediaSet(strName, &parent, nID, E_MediaSetType::MST_SingerGroup)
{
}
