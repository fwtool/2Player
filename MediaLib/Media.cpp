
#include <MediaDef.h>

wstring CMedia::GetBaseDir() const
{
	if (NULL != m_pParent)
	{
		return m_pParent->GetBaseDir();
	}
	
	return L"";
}

wstring CMedia::GetDurationString() const
{
	if (m_iDuration > 0)
	{
		WCHAR pszFileSize[16]{ 0 };
		wsprintf(pszFileSize, L"%u:%02u", m_iDuration / 60, m_iDuration % 60);
		return pszFileSize;
	}

	return L"";
}

//void CMedia::UpdateFileInfo(int iFileSize, int iDuration)
//{
//	m_iFileSize = iFileSize;
//
//	m_iDuration = iDuration;
//}

void CMedia::OnListItemRename(const wstring& strNewName)
{
	__Ensure(g_pRootMediaRes);
	g_pRootMediaRes->m_MediaObserver.renameMedia(*this, strNewName);
}

void CMedia::onAsyncTask()
{
	CheckFileSize();
	
	m_iDuration = CheckDuration();
}

void CMedia::_UpdatePath(const wstring& strPath)
{
	fsutil::SplitPath(strPath, &m_strParentDir, &m_strName);

	m_strTitle = fsutil::getFileTitle(m_strName);
}

void CMedia::UpdatePath(const wstring& strPath)
{
	_UpdatePath(strPath);
	
	updateFileType(strPath);
		
	onAsyncTask();
}

CMediaRes *CMedia::getMediaRes() const
{
	__EnsureReturn(g_pRootMediaRes, NULL);
	return g_pRootMediaRes->FindSubPath(GetPath(), false);
}

CRCueFile CMedia::getCueFile() const
{
	CMediaRes *pMediaRes = getMediaRes();
	__EnsureReturn(pMediaRes, CCueFile::NoCue);

	return pMediaRes->getCueFile();
}
