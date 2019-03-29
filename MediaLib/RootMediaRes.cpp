
#include <MediaDef.h>

static const vector<wstring> g_vecMediaExtName = { L"mp3", L"flac", L"wav", L"ape", L"wma" };

static map<wstring, E_MediaFileType> g_mapFileType;

CRootMediaRes *g_pRootMediaRes = NULL;

E_MediaFileType CRootMediaRes::GetMediaFileType(const wstring& strExtName)
{
	auto itr = g_mapFileType.find(util::StrLowerCase(strExtName));
	if (itr == g_mapFileType.end())
	{
		return E_MediaFileType::MFT_Null;
	}

	return itr->second;
}

wstring CRootMediaRes::GetMediaFileType(E_MediaFileType eMediaFileType)
{
	if (E_MediaFileType::MFT_Null == eMediaFileType)
	{
		return L"";
	}

	return g_vecMediaExtName[(UINT)eMediaFileType - 1];
}

CRootMediaRes::CRootMediaRes(IMediaObserver& MediaObserver)
	: m_MediaObserver(MediaObserver)
{
	UINT uFileType = 0;
	for (auto& strMediaExtName : g_vecMediaExtName)
	{
		g_mapFileType[fsutil::dot + strMediaExtName] = (E_MediaFileType)++uFileType;
	}

	g_pRootMediaRes = this;
}

void CRootMediaRes::init(const wstring& strDir, const vector<wstring>& vecAttachDir)
{
	m_vecAttachDir = vecAttachDir;
	m_mapAttachDir.clear();
	
	CPath::SetDir(strDir);
}

TD_PathList& CRootMediaRes::_findFile()
{
	if (NULL != m_plstSubPath)
	{
		return *m_plstSubPath;
	}

    TD_PathList& arrSubPath = CMediaRes::_findFile();

	wstring strDirName;
	for (auto& strAttachDir : m_vecAttachDir)
	{
		auto pAttachDir = new CAttachDir(strAttachDir);
		arrSubPath.add(pAttachDir);
		
		strDirName = util::StrLowerCase(fsutil::GetFileName(strAttachDir));
		m_mapAttachDir[strDirName] = pAttachDir;
	}
	
	return arrSubPath;
}

wstring CRootMediaRes::toAbsPath(const wstring& strSubPath) const
{
	auto strAbsPath = m_strName + strSubPath;

	if (!m_mapAttachDir.empty())
	{
		auto pos = strSubPath.find_first_of(fsutil::backSlant, 1);
		if (wstring::npos != pos)
		{
			wstring strSubDirName = util::StrLowerCase(strSubPath.substr(1, pos-1));
			auto itr = m_mapAttachDir.find(strSubDirName);
			if (itr != m_mapAttachDir.end())
			{
				if (!fsutil_win::ExistsFile(strAbsPath))
				{
					strAbsPath = itr->second->GetAbsPath() + strSubPath.substr(strSubDirName.size() + 1);
				}
			}
		}
	}

	return strAbsPath;
}

wstring CRootMediaRes::toOppPath(const wstring& strAbsPath) const
{
	return fsutil::GetOppPath(strAbsPath, m_strName);
}

CMediaRes* CRootMediaRes::FindSubPath(wstring strSubPath, bool bDir)
{
	if (strSubPath.empty())
	{
		return this;
	}

	if (!m_mapAttachDir.empty())
	{
		wstring strDirName;
		auto pos = strSubPath.find_first_of(fsutil::backSlant, 1);
		if (wstring::npos != pos)
		{
			strDirName = strSubPath.substr(1, pos-1);
		}
		else
		{
			strDirName = strSubPath.substr(1);
		}
		util::StrLowerCase(strDirName);
		
		auto itr = m_mapAttachDir.find(strDirName);
		if (itr != m_mapAttachDir.end())
		{
			if (wstring::npos != pos)
			{
				auto pMediaRes = itr->second->FindSubPath(strSubPath.substr(pos), bDir);
				if (NULL != pMediaRes)
				{
					return pMediaRes;
				}
			}
			else
			{
				return itr->second;
			}
		}
	}

    return CMediaRes::FindSubPath(strSubPath, bDir);
}

bool CRootMediaRes::checkIndependentDir(const wstring& strAbsDir, bool bCheckAttachDir)
{
	const auto& strPath = m_strName;
	if (util::StrMatchIgnoreCase(strPath, strAbsDir))
	{
		return false;
	}

	if (fsutil::CheckSubPath(strPath, strAbsDir))
	{
		return false;
	}

	if (fsutil::CheckSubPath(strAbsDir, strPath))
	{
		return false;
	}

	if (bCheckAttachDir)
	{
		for (auto& pr : m_mapAttachDir)
		{
			auto strAttachDir = pr.second->GetAbsPath();
			if (util::StrMatchIgnoreCase(strAttachDir, strAbsDir))
			{
				return false;
			}

			if (fsutil::CheckSubPath(strAttachDir, strAbsDir))
			{
				return false;
			}

			if (fsutil::CheckSubPath(strAbsDir, strAttachDir))
			{
				return false;
			}
		}
	}

	return true;
}
