
#include <model.h>

#include <AutoMatch.h>

static bool matchTitle(const wstring& strTitle1, const wstring& strTitle2)
{
	if (strTitle1.size() == 1 || strTitle2.size() == 1)
	{
		return strTitle1 == strTitle2;
	}

	if (wstring::npos != strTitle1.find(strTitle2) || wstring::npos != strTitle2.find(strTitle1))
	{
		return true;
	}

	return false;
}

static UINT matchTitle(const SSet<wstring>& setFileTitle1, const SSet<wstring>& setFileTitle2)
{
	UINT uMatchCount = 0;

	SVector<wstring> vecFileTitle2(setFileTitle2);
	for (auto& strTitle1 : setFileTitle1)
	{
		for (auto itr = vecFileTitle2.begin(); itr != vecFileTitle2.end(); )
		{
			if (matchTitle(strTitle1, *itr))
			{
				uMatchCount++;
				itr = vecFileTitle2.erase(itr);
				break;
			}
			else
			{
				++itr;
			}
		}
	}

	return uMatchCount;
}

static UINT matchTitle(const wstring& strFileTitle1, const SSet<wstring>& setFileTitle2)
{
	UINT uMatchCount = 0;
	for (auto& strTitle2 : setFileTitle2)
	{
		if (matchTitle(strFileTitle1, strTitle2))
		{
			uMatchCount++;
		}
	}
	return uMatchCount;
}

CSearchMediaInfoGuard::CSearchMediaInfoGuard(CSingerMgr& SingerMgr)
	: m_SingerMgr(SingerMgr)
{
	TD_MediaSetList lstSingers;
	m_SingerMgr.GetAllSinger(lstSingers);

	m_setSingerName = lstSingers.map([](CMediaSet& Singer) {
		return util::StrLowerCase(Singer.m_strName);
	});
}

void CSearchMediaInfoGuard::genSearchMediaInfo(CMedia& Media, TD_SearchMediaInfoMap& mapSearchMediaInfo)
{
	wstring strMediaPath = util::StrLowerCase(Media.GetPath());

	auto itr = mapSearchMediaInfo.find(strMediaPath);
	if (itr != mapSearchMediaInfo.end())
	{
		itr->second.m_lstMedias.add(Media);
	}
	else
	{
		CSinger *pSinger = NULL;
		if (E_MediaSetType::MST_Album == Media.GetMediaSetType())
		{
			pSinger = ((CAlbumItem&)Media).GetSinger();
		}
		else
		{
			pSinger = m_SingerMgr.GetSinger(Media.GetPath());
		}

		CSearchMediaInfo& SearchMediaInfo = mapSearchMediaInfo[strMediaPath];
		if (NULL != pSinger)
		{
			SearchMediaInfo = CSearchMediaInfo(Media, *pSinger);
		}
		else
		{
			SearchMediaInfo = CSearchMediaInfo(Media, m_setSingerName);
		}

		SearchMediaInfo.m_lstMedias.add(Media);
	}
}

CSearchMediaInfo::CSearchMediaInfo(CMedia& Media, CSinger& singer)
	: m_FileTitle(Media.GetTitle())
	, m_strAbsPath(Media.GetAbsPath())
	, m_strSingerName(util::StrLowerCase(singer.m_strName))
{
	m_FileTitle.setFileTitle.del(m_strSingerName);
}

CSearchMediaInfo::CSearchMediaInfo(CMedia& Media, const SSet<wstring>& setSingerName)
	: m_FileTitle(Media.GetTitle())
	, m_strAbsPath(Media.GetAbsPath())
{
    m_FileTitle.setFileTitle.del_ex([&](const wstring& strTitle) {
		if (setSingerName.includes(strTitle))
		{
			m_strSingerName = strTitle;
			return E_DelConfirm::DC_YesAbort;
		}
		return E_DelConfirm::DC_No;
	});
}

wstring CSearchMediaInfo::GetFileSize()
{
	if (m_strFileSize.empty())
	{
		int iFileSize = fsutil::GetFileSize(m_strAbsPath);
		if (iFileSize > 0)
		{
			m_strFileSize = IMedia::GetFileSizeString(iFileSize);
		}
		else
		{
			m_strFileSize = L'0';
		}
	}

	return m_strFileSize;
}

bool CSearchMediaInfo::matchMediaRes(tagMediaResInfo& MediaResInfo)
{
	auto& setMediaTitle = m_FileTitle.setFileTitle;
	SSet<wstring> setMediaResTitle;
	
	if (!m_strSingerName.empty())
	{
		for (auto& strMediaResTitle : MediaResInfo.FileTitle.setFileTitle)
		{
			if (strMediaResTitle != m_strSingerName)
			{
				setMediaResTitle.add(strMediaResTitle);
			}
		}
	}
	else
	{
		setMediaResTitle = MediaResInfo.FileTitle.setFileTitle;
	}

	UINT uMatchCount = 0;
	if (setMediaTitle.size() > 1 && setMediaResTitle.size() > 1)
	{
		uMatchCount = matchTitle(setMediaTitle, setMediaResTitle);
	}
	else if (setMediaTitle.size() > 1)
	{
        setMediaResTitle.getFront([&](const wstring& strMediaResTitle) {
			uMatchCount = matchTitle(strMediaResTitle, setMediaTitle);
		});
	}
	else if (setMediaResTitle.size() > 1)
	{
        setMediaTitle.getFront([&](const wstring& strMediaTitle) {
			uMatchCount = matchTitle(strMediaTitle, setMediaResTitle);
		});
	}
	else
	{
		bool bRet = false;
        setMediaResTitle.getFront([&](const wstring& strMediaResTitle) {
            setMediaTitle.getFront([&](const wstring& strMediaTitle) {
				bRet = matchTitle(strMediaResTitle, strMediaTitle);
			});
		});
		return bRet;
	}

	if (!m_strSingerName.empty())
	{
		if (wstring::npos != MediaResInfo.FileTitle.strFileTitle.find(m_strSingerName))
		{
			return uMatchCount >= 1;
		}
	}
	
	return uMatchCount >= 2;
}

void CAutoMatch::autoMatchMedia(CSingerMgr& SingerMgr, CMediaRes& SrcPath, const TD_MediaList& lstMedias)
{
	CSearchMediaInfoGuard SearchMediaInfoGuard(SingerMgr);

	set<wstring> setPaths;
	set<wstring> setSubPaths;

	wstring strBaseDir;
	lstMedias([&](CMedia& Media) {
		strBaseDir = util::StrLowerCase(Media.GetBaseDir());

		if (!strBaseDir.empty())
		{
			if (&SrcPath != &m_RootMediaRes)
			{
				if (util::StrMatchIgnoreCase(strBaseDir, SrcPath.GetPath())
					|| fsutil::CheckSubPath(strBaseDir, SrcPath.GetPath()))
				{
					setPaths.insert(strBaseDir);
				}
				else if (fsutil::CheckSubPath(SrcPath.GetPath(), strBaseDir))
				{
					setSubPaths.insert(strBaseDir);
				}
				else
				{
					return;
				}
			}
			else
			{
				setSubPaths.insert(strBaseDir);
			}
		}
		else
		{
			setPaths.insert(L"");
		}

		SearchMediaInfoGuard.genSearchMediaInfo(Media, m_mapSearchMedia[strBaseDir]);
	});

	if (!setPaths.empty() || !setSubPaths.empty())
	{
		list<wstring> lstPaths(setPaths.begin(), setPaths.end());
		list<wstring> lstSubPaths(setSubPaths.begin(), setSubPaths.end());

		enumMediaRes(SrcPath, &SrcPath, lstPaths, lstSubPaths);
	}
}

void CAutoMatch::enumMediaRes(CMediaRes& SrcPath, CMediaRes *pDir, list<wstring>& lstPaths, list<wstring>& lstSubPaths)
{
	__Ensure(m_cbProgress(pDir->GetAbsPath()));

	if (!lstPaths.empty())
	{
		TD_PathList lstMediaRes;
		pDir->GetSubFile(lstMediaRes);
		lstMediaRes([&](CPath& Path) {
			CMediaRes& MediaRes = (CMediaRes&)Path;
			tagMediaResInfo MediaResInfo(MediaRes.GetPath(), MediaRes.GetFileSizeString());

			for (list<wstring>::iterator itPath = lstPaths.begin(); itPath != lstPaths.end(); )
			{
				auto itr = m_mapSearchMedia.find(*itPath);
				if (itr != m_mapSearchMedia.end())
				{
					TD_SearchMediaInfoMap& mapSearchMediaInfo = itr->second;
					matchMedia(MediaRes, MediaResInfo, mapSearchMediaInfo);
					if (mapSearchMediaInfo.empty())
					{
						m_mapSearchMedia.erase(itr);
						if (m_mapSearchMedia.empty())
						{
							return false;
						}

						itPath = lstPaths.erase(itPath);

						continue;
					}
				}

				itPath++;
			}

			if (lstPaths.empty())
			{
				return false;
			}

			return true;
		});
	}
	
	if (m_mapSearchMedia.empty())
	{
		return;
	}

	if (!lstPaths.empty() || !lstSubPaths.empty())
	{
		TD_PathList lstSubDir;
		pDir->GetSubDir(lstSubDir);

		lstSubDir([&](CPath& SubDir) {
			list<wstring> lstNewPaths = lstPaths;
			list<wstring> lstNewSubPaths;
			FilterBasePath(SrcPath, (CMediaRes*)&SubDir, lstSubPaths, lstNewPaths, lstNewSubPaths);

			if (lstNewPaths.empty() && lstNewSubPaths.empty())
			{
				if (lstSubPaths.empty())
				{
					return false;
				}
				return true;
			}

			enumMediaRes(SrcPath, (CMediaRes*)&SubDir, lstNewPaths, lstNewSubPaths);
		
			return true;
		});
	}
}

void CAutoMatch::FilterBasePath(CMediaRes& SrcPath, CMediaRes *pDir, list<wstring>& lstSubPaths
	, list<wstring>& lstPaths, list<wstring>& lstNewSubPaths)
{
	wstring strPath = pDir->GetPath();

	BOOL bFlag = FALSE;

	for (list<wstring>::iterator itSubPath = lstSubPaths.begin()
		; itSubPath != lstSubPaths.end(); )
	{
		bFlag = FALSE;

		if (util::StrMatchIgnoreCase(*itSubPath, strPath))
		{
			lstPaths.push_back(*itSubPath);

			bFlag = TRUE;
		}
		else if (fsutil::CheckSubPath(strPath, *itSubPath))
		{			
			lstNewSubPaths.push_back(*itSubPath);

			bFlag = TRUE;
		}
		else if (pDir == &SrcPath)
		{
			if (&SrcPath != &m_RootMediaRes)
			{
				if (fsutil::CheckSubPath(*itSubPath, strPath))
				{
					lstPaths.push_back(*itSubPath);

					bFlag = TRUE;
				}
			}
		}

		if (bFlag)
		{
			itSubPath = lstSubPaths.erase(itSubPath);
			continue;
		}

		itSubPath++;
	}

	for (list<wstring>::iterator itPath = lstPaths.begin()
		; itPath != lstPaths.end(); )
	{
		if (m_mapSearchMedia.find(*itPath) == m_mapSearchMedia.end())
		{
			itPath = lstPaths.erase(itPath);
			continue;
		}

		itPath++;
	}
}

void CAutoMatch::matchMedia(CMediaRes& MediaRes, tagMediaResInfo& MediaResInfo, TD_SearchMediaInfoMap& mapSearchMediaInfo)
{
	for (auto itr = mapSearchMediaInfo.begin(); itr != mapSearchMediaInfo.end(); )
	{
		auto& SearchMediaInfo = itr->second;

		if (SearchMediaInfo.matchMediaRes(MediaResInfo))
		{
			E_MatchResult eRet = m_cbConfirm(SearchMediaInfo, MediaResInfo);
			if (E_MatchResult::MR_Ignore == eRet)
			{
				itr = mapSearchMediaInfo.erase(itr);
				continue;
			}
			else if (E_MatchResult::MR_Yes == eRet)
			{
				mapSearchMediaInfo.erase(itr);
				break;
			}
		}

		itr++;
	}
}
