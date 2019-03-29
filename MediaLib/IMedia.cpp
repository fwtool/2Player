
#include <MediaDef.h>

IMedia::IMedia(const wstring& strPath)
{
	updateFileType(strPath);
}

IMedia::IMedia(E_MediaFileType eFileType)
{
	m_eFileType = eFileType;
}

void IMedia::updateFileType(const wstring& strPath)
{
	m_eFileType = CRootMediaRes::GetMediaFileType(fsutil::GetFileExtName(strPath));
}

wstring IMedia::GetFileTypeString() const
{
	return CRootMediaRes::GetMediaFileType(m_eFileType);
}

wstring IMedia::GetExtName() const
{
	wstring strFileType = CRootMediaRes::GetMediaFileType(m_eFileType);
	if (!strFileType.empty())
	{
		return L"." + strFileType;
	}
	else
	{
		return L"";
	}
}

wstring IMedia::GetFileSizeString(int iFileSize, bool bIgnoreByte)
{
	__EnsureReturn(iFileSize > 0, L"");

	LONG nK = (LONG)(iFileSize / 1000);

	int nM = nK / 1000;
	nK %= 1000;

	wchar_t lpBuff[64]{0};
	if (bIgnoreByte)
	{
		if (5 == swprintf(lpBuff, sizeof(lpBuff), L"%d,%03d", nM, nK))
		{
			return wstring(L" ") + lpBuff;
		}
	}
	else
	{
		swprintf(lpBuff, sizeof(lpBuff), L"%d,%03d,%03d", nM, nK, iFileSize%1000);
	}

	return lpBuff;
}

wstring IMedia::GetFileSizeString(bool bIgnoreByte) const
{
	__EnsureReturn(!IsDir(), L"");

	return GetFileSizeString(GetFileSize(), bIgnoreByte);
}

int IMedia::CheckDuration()
{
	__EnsureReturn(g_pRootMediaRes, -1);
	return g_pRootMediaRes->m_MediaObserver.checkMedia(this->GetAbsPath());
}

E_MediaSetType IMedia::GetMediaSetType()
{
    CMediaSet *pMediaSet = GetMediaSet();
    __EnsureReturn(pMediaSet, E_MediaSetType::MST_Null);

    return pMediaSet->m_eType;
}

bool IMedia::DoAsyncTask()
{
    if (!m_bAsyncTaskFlag)
    {
        return false;
    }
    m_bAsyncTaskFlag = false;

    onAsyncTask();

    return true;
}

void IMedia::SetRelatedMediaSet(E_MediaSetType eMediaSetType, int iMediaSetID, const wstring& strMediaSetName, int iMediaID)
{
    if (E_MediaSetType::MST_Null != eMediaSetType)
    {
        tagRelatedMediaSet& RelatedMediaSet = _getRelatedMediaSet(eMediaSetType);

        RelatedMediaSet.iMediaSetID = iMediaSetID;
        RelatedMediaSet.strMediaSetName = strMediaSetName;

        RelatedMediaSet.iMediaID = iMediaID;
    }
}

void IMedia::SetRelatedMediaSet(CMediaSet& MediaSet)
{
    SetRelatedMediaSet(MediaSet.m_eType, MediaSet.m_uID, MediaSet.m_strName);
}

void IMedia::SetRelatedMediaSet(CMedia& Media)
{
    auto pMediaSet = Media.m_pParent;
    if (NULL != pMediaSet)
    {
        SetRelatedMediaSet(pMediaSet->m_eType, pMediaSet->m_uID, pMediaSet->m_strName, Media.m_uID);
    }
}

void IMedia::ClearRelatedMediaSet(E_MediaSetType eMediaSetType)
{
    if (E_MediaSetType::MST_Null != eMediaSetType)
    {
        _getRelatedMediaSet(eMediaSetType).clear();
    }
}

bool IMedia::UpdateRelatedMediaSet(const tagMediaSetChanged& MediaSetChanged)
{
    if (CheckRelatedMediaSetChanged(MediaSetChanged)
        || GetRelatedMediaSetID(MediaSetChanged.eMediaSetType) == MediaSetChanged.uMediaSetID)
    {
        onAsyncTask();

        return true;
    }

    return false;
}
