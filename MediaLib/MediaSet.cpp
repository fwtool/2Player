
#include <MediaDef.h>

UINT CMediaSet::GetIndex() const
{
	if (NULL == m_pParent)
	{
		return 0;
	}

	TD_MediaSetList lstSubSets;
	m_pParent->GetSubSets(lstSubSets);

	int iPos = lstSubSets.indexOf(this);
	if (iPos < 0)
	{
		iPos = 0;
	}

	return iPos+1;
}

wstring CMediaSet::GetBaseDir() const
{
	if (NULL != m_pParent)
	{
		return m_pParent->GetBaseDir();
	}

	return L"";
}

BOOL CMediaSet::RemoveMedia(CMedia& Media)
{
	return 0 != m_lstMedias.del(Media);
}

void CMediaSet::GetAllMediaSets(E_MediaSetType eType, TD_MediaSetList& lstMediaSets)
{
	TD_MediaSetList lstSubSets;
	this->GetSubSets(lstSubSets);
	
	lstSubSets([&](CMediaSet& SubSet) {
		if (0 != SubSet.m_uID)
		{
			if (SubSet.m_eType == eType || E_MediaSetType::MST_Null == eType)
			{
				lstMediaSets.add(SubSet);
			}
		}

		SubSet.GetAllMediaSets(eType, lstMediaSets);
	});
}

void CMediaSet::GetAllMedias(TD_MediaList& lstMedias)
{
	this->GetMedias(lstMedias);
	
	TD_MediaSetList lstSubSets;
	this->GetSubSets(lstSubSets);
	lstSubSets([&](CMediaSet& SubSet) {
		SubSet.GetAllMedias(lstMedias);
	});
}

CMediaSet* CMediaSet::GetMediaSet(UINT uMediaSetID, E_MediaSetType eMediaSetType)
{
	if (E_MediaSetType::MST_Null == eMediaSetType || m_eType == eMediaSetType)
	{
		if (m_uID == uMediaSetID)
		{
			return this;
		}
	}

	CMediaSet *pRetMediaSet = NULL;
	TD_MediaSetList lstSubSets;
	GetSubSets(lstSubSets);
	lstSubSets([&](CMediaSet& SubSet) {
		pRetMediaSet = SubSet.GetMediaSet(uMediaSetID, eMediaSetType);
		if (NULL != pRetMediaSet)
		{
			return false;
		}

		return true;
	});

	return pRetMediaSet;
}

CMedia* CMediaSet::GetMedia(UINT uMediaID, E_MediaSetType eMediaSetType)
{
	CMedia *pMedia = NULL;

	if (E_MediaSetType::MST_Null == eMediaSetType || m_eType == eMediaSetType)
	{
		TD_MediaList lstMedias;
		GetMedias(lstMedias);
		
		lstMedias([&](CMedia& Media) {
			if (Media.m_uID == uMediaID)
			{
				pMedia = &Media;
				return false;
			}

			return true;
		});
		if (NULL != pMedia)
		{
			return pMedia;
		}
	}

	TD_MediaSetList lstSubSets;
	GetSubSets(lstSubSets);
	lstSubSets([&](CMediaSet& SubSet) {
		pMedia = SubSet.GetMedia(uMediaID, eMediaSetType);
		if (NULL != pMedia)
		{
			return false;
		}

		return true;
	});

	return pMedia;
}

wstring CMediaSet::GetLogicPath(const wstring& strDelimiter)
{
	wstring strLogicPath;
	if (NULL != m_pParent)
	{
		strLogicPath = m_pParent->GetLogicPath(strDelimiter) + strDelimiter + m_strName;
	}
	else
	{
		strLogicPath = m_strName;
	}

	return strLogicPath;
}

void CMediaSet::GetTreeChilds(TD_TreeObjectList& lstChilds)
{
	TD_MediaSetList lstSubSets;
	this->GetSubSets(lstSubSets);

	lstChilds.add(lstSubSets);
}

bool CMediaSet::SetAsyncTaskFlag()
{
	TD_MediaList lstMedias;
	this->GetMedias(lstMedias);
	if (!lstMedias)
	{
		return false;
	}

	lstMedias([](CMedia& Media) {
		Media.SetAsyncTaskFlag();
	});

	return true;
}

bool CMediaSet::FindMedia(const tagFindMediaPara& FindPara, tagFindMediaResult& FindResult)
{
	__EnsureReturn(g_pRootMediaRes, false);

	bool bRet = true;
	if (E_MediaSetType::MST_Null == FindPara.eFindMediaSetType || this->m_eType == FindPara.eFindMediaSetType)
	{
		TD_MediaList lstMedias;
		this->GetMedias(lstMedias);

		lstMedias([&](CMedia& Media) {
			return bRet = _FindMedia(Media, FindPara, FindResult);
		});
		if (!bRet)
		{
			return false;
		}
	}

	TD_MediaSetList lstSubSets;
	this->GetSubSets(lstSubSets);
	lstSubSets([&](CMediaSet& SubSet) {
		return bRet = SubSet.FindMedia(FindPara, FindResult);
	});

	return bRet;
}

bool CMediaSet::_FindMedia(CMedia& Media, const tagFindMediaPara& FindPara, tagFindMediaResult& FindResult)
{
	bool bFlag = false;

	wstring strPath = Media.GetPath();

	switch (FindPara.eFindMediaMode)
	{
	case E_FindMediaMode::FMM_MatchText:
		if (wstring::npos != util::StrLowerCase(Media.GetTitle()).find(FindPara.strFindText))
		{
			bFlag = true;
		}

		break;
	case E_FindMediaMode::FMM_MatchPath:
		if (strPath == FindPara.strFindText)
		{
			bFlag = true;
		}

		break;
	case E_FindMediaMode::FMM_MatchDir:
		if (fsutil::CheckSubPath(FindPara.strFindText, strPath))
		{
			bFlag = true;
			break;
		}

		if (!FindPara.strFindSingerName.empty())
		{
			if (wstring::npos != util::StrLowerCase(Media.GetTitle()).find(FindPara.strFindSingerName))
			{
				bFlag = true;
			}
		}

		break;

	case E_FindMediaMode::FMM_RenameDir:
		bFlag = fsutil::CheckSubPath(FindPara.strDir, strPath);

		break;
	case E_FindMediaMode::FMM_MatchFiles:
		if (FindPara.setFiles.find(strPath) != FindPara.setFiles.end())
		{
			bFlag = true;
		}

		break;
	default:
		return false;
	}

	if (bFlag)
	{
		FindResult.lstRetMedias.add(Media);

		FindResult.uResult++;

		if (FindPara.bFindOne)
		{
			return false;
		}
	}

	return true;
}
