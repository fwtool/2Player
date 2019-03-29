
#include <MediaDef.h>

#include "MP3ID3.h"

CMediaRes::CMediaRes(const tagFindData& findData, CPath *pParentDir, E_MediaFileType eFileType)
	: CPathObject(findData, pParentDir)
	, IMedia(eFileType)
{
}

CMediaRes::CMediaRes(const wstring& strDir)
	: CPathObject(strDir, true)
	, IMedia(E_MediaFileType::MFT_Null)
{
}

bool CMediaRes::_loadCue(const wstring& strFileName)
{
	CCueFile cueFile(strFileName);
	if (!cueFile.load(GetAbsPath() + fsutil::backSlant + strFileName))
	{
		return false;
	}

	m_alSubCueFile.add(cueFile);

	if (cueFile.m_strMediaFileName.empty())
	{
		return false;
	}

	wstring strMediaFileName = util::StrLowerCase(cueFile.m_strMediaFileName);
	wstring strExtName = fsutil::GetFileExtName(strMediaFileName);
	if (strExtName.empty())
	{
		return false;
	}

	m_mapSubCueFile.insert(strMediaFileName, m_alSubCueFile.size() - 1);

	strMediaFileName = util::StrLowerCase(strFileName.substr(0, strFileName.size() - 4)) + strExtName;
	m_mapSubCueFile.insert(strMediaFileName, m_alSubCueFile.size() - 1);

	return true;
}

CRCueFile CMediaRes::getSubCueFile(CMediaRes& MediaRes)
{
	LPCCueFile pCueFile = CCueFile::NoCue;
	
	m_mapSubCueFile.get(util::StrLowerCase(MediaRes.GetName()), [&](UINT uIdx) {
        m_alSubCueFile.get(uIdx, [&](CCueFile& cueFile) {
			if (cueFile.m_alTrackInfo)
			{
				pCueFile = &cueFile;
			}
		});
	});

	return *pCueFile;
}

CRCueFile CMediaRes::getCueFile()
{
	if (NULL == m_pCueFile)
	{
		if (NULL != m_pParentDir)
		{
			m_pCueFile = ((CMediaRes*)m_pParentDir)->getSubCueFile(*this);
		}
		else
		{
			m_pCueFile = CCueFile::NoCue;
		}
	}

	return *m_pCueFile;
}

wstring CMediaRes::GetPath() const
{
	if (NULL != m_pParentDir)
	{
		return ((CMediaRes*)m_pParentDir)->GetPath() + fsutil::backSlant + m_strName;
	}
	
	return L"";
}

wstring CMediaRes::GetAbsPath() const
{
	if (NULL != m_pParentDir)
	{
		return ((CMediaRes*)m_pParentDir)->GetAbsPath() + fsutil::backSlant + m_strName;
	}

	return m_strName;
}

CPath *CMediaRes::NewSubPath(const tagFindData& findData, CPath *pParentDir)
{
	if (findData.isDir())
	{
		return new CMediaRes(findData, pParentDir);
	}
	else
	{
		__EnsureReturn(g_pRootMediaRes, NULL);

		wstring strExtName = fsutil::GetFileExtName(findData.getFileName());
		auto eFileType = CRootMediaRes::GetMediaFileType(strExtName);
		if (E_MediaFileType::MFT_Null == eFileType)
		{
			if (L".cue" == strExtName)
			{
				_loadCue(findData.getFileName());

				return NULL;
			}

			return NULL;
		}

		return new CMediaRes(findData, pParentDir, eFileType);
	}
}

int CMediaRes::_getImage()
{
	int iImage = 0;

	if (m_bDir)
	{
		if (GetParentDir().empty())
		{
			iImage = (int)E_GlobalImage::GI_FolderLink;
		}
		else
		{
			iImage = (int)E_GlobalImage::GI_Folder;

			int iRelatedSingerID = GetRelatedMediaSetID(E_MediaSetType::MST_Singer);
			if (iRelatedSingerID > 0)
			{
				__EnsureReturn(g_pRootMediaRes, -1);
				UINT uSingerImgPos = g_pRootMediaRes->m_MediaObserver.getSingerImgPos((UINT)iRelatedSingerID);
				if ((UINT)E_GlobalImage::GI_SingerDefault != uSingerImgPos)
				{
					iImage = uSingerImgPos;
				}
			}
		}
	}
	else
	{
		if (getCueFile())
		{
			iImage = (int)E_GlobalImage::GI_DiskFile;
		}
		else
		{
			iImage = (int)E_GlobalImage::GI_MediaFile;
		}
	}

	return iImage;
}

void CMediaRes::GenListItem(vector<wstring>& vecText, int& iImage, bool bGenRelatedSinger)
{
	iImage = _getImage();

	vecText.push_back(GetName());

	if (m_bDir)
	{
		UINT uSubPathCount = GetSubPathCount();
		if (0 == uSubPathCount)
		{
			vecText.push_back(L"");
		}
		else
		{
			vecText.push_back((std::to_wstring(uSubPathCount) + L"项"));
		}
	}
	else
	{
		vecText.push_back(GetFileSizeString());
	}
	
	vecText.push_back(m_bDir ? L"" : GetRelatedMediaSetName(E_MediaSetType::MST_Playlist));
	
	if (bGenRelatedSinger)
	{
		wstring strSingerAlbum = GetRelatedMediaSetName(E_MediaSetType::MST_Singer);
		if (!m_bDir)
		{
			wstring strAlbum = GetRelatedMediaSetName(E_MediaSetType::MST_Album);
			if (!strAlbum.empty())
			{
				strSingerAlbum.append(__CNDotDelimiter + strAlbum);
			}
		}
		vecText.push_back(strSingerAlbum);
	}
	else
	{
		vecText.push_back(GetRelatedMediaSetName(E_MediaSetType::MST_Album));
	}
		
	vecText.push_back(util::FormatTime(m_modifyTime, __TimeFormat));
	
	vecText.push_back(__Space + m_MediaTag.strTitle);
	vecText.push_back(__Space + m_MediaTag.strArtist);
	vecText.push_back(__Space + m_MediaTag.strAlbum);
}

bool CMediaRes::GetRenameText(wstring& stRenameText) const
{
	if (m_bDir)
	{
		stRenameText = GetName();
	}
	else
	{
		stRenameText = fsutil::getFileTitle(this->GetPath());
	}
	return true;
}

void CMediaRes::OnListItemRename(const wstring& strNewName)
{
	__Ensure(g_pRootMediaRes);
	g_pRootMediaRes->m_MediaObserver.renameMedia(*this, strNewName);
}

void CMediaRes::SetDirRelatedSinger(UINT uSingerID, const wstring& strSingerName, bool& bChanged)
{
	SetRelatedMediaSet(E_MediaSetType::MST_Album, uSingerID, strSingerName, bChanged);
}

bool CMediaRes::ReadTag()
{
	if (E_MediaFileType::MFT_MP3 == m_eFileType)
	{
		FILE *lpFile = NULL;
		(void)_wfopen_s(&lpFile, this->GetAbsPath().c_str(), L"rb+");
		__EnsureReturn(lpFile, false);

		ReadMP3Tag(lpFile);

		(void)fclose(lpFile);
	
		return true;
	}
	else if (E_MediaFileType::MFT_FLAC == m_eFileType)
	{
		return ReadFlacTag();
	}
}

void CMediaRes::ReadMP3Tag(FILE *lpFile)
{
	m_MediaTag.strTitle.clear();
	m_MediaTag.strArtist.clear();
	m_MediaTag.strAlbum.clear();

	MP3ID3V1INFO ID3V1INFO;
	memset(&ID3V1INFO, 0, sizeof(ID3V1INFO));
	if (0 == AudioInfo_GetMP3ID3V1(lpFile, ID3V1INFO) && ID3V1INFO.bHasTag)
	{
		if (*ID3V1INFO.Title)
		{
			m_MediaTag.strTitle = util::StrToWStr(ID3V1INFO.Title);
		}

		if (*ID3V1INFO.Artist)
		{
			m_MediaTag.strArtist = util::StrToWStr(ID3V1INFO.Artist);
		}

		if (*ID3V1INFO.Album)
		{
			m_MediaTag.strAlbum = util::StrToWStr(ID3V1INFO.Album);
		}
	}

	if (m_MediaTag.strTitle.empty() || m_MediaTag.strArtist.empty() || m_MediaTag.strAlbum.empty())
	{
		MP3ID3V2INFO ID3V2INFO;
		memset(&ID3V2INFO, 0, sizeof(ID3V2INFO));
		if (0 == AudioInfo_GetMP3ID3V2(lpFile, ID3V2INFO) && ID3V2INFO.bHasTag)
		{
			if (m_MediaTag.strTitle.empty())
			{
				string& strTitle = ID3V2INFO.ppszTagInfo[ID3FID_TITLE];
				if (!strTitle.empty())
				{
					m_MediaTag.strTitle = util::StrToWStr(strTitle, util::IsUTF8Str(strTitle) ? CP_UTF8 : CP_ACP);
				}
			}
			
			if (m_MediaTag.strArtist.empty())
			{
				string& strArtist = ID3V2INFO.ppszTagInfo[ID3FID_LEADARTIST];
				if (!strArtist.empty())
				{
					m_MediaTag.strArtist = util::StrToWStr(strArtist, util::IsUTF8Str(strArtist) ? CP_UTF8 : CP_ACP);
				}
			}

			if (m_MediaTag.strAlbum.empty())
			{
				string& strAlbum = ID3V2INFO.ppszTagInfo[ID3FID_ALBUM];
				if (!strAlbum.empty())
				{
					m_MediaTag.strAlbum = util::StrToWStr(strAlbum, util::IsUTF8Str(strAlbum) ? CP_UTF8 : CP_ACP);
				}
			}
		}
	}
}

static inline bool findFlacTag(const char *lpData, const char* pszFind, UINT size, wstring& strRet)
{
	if (0 == _strnicmp(lpData, pszFind, size))
	{
		string strData = lpData + size;
		strData.pop_back();
		strRet = util::StrToWStr(strData, util::IsUTF8Str(strData) ? CP_UTF8 : CP_ACP);

		return true;
	}

	return false;
}

#define __MaxSize_FlacTag 2048

static const char __TitleField[] = "Title=";
static const char __ArtistField[] = "Artist=";
static const char __AlbumField[] = "Album=";

bool CMediaRes::ReadFlacTag()
{
	vector<char> vecBuffer;
	if (!fsutil::loadBinary(this->GetAbsPath(), vecBuffer, __MaxSize_FlacTag))
	{
		return false;
	}
	if (vecBuffer.size() < __MaxSize_FlacTag)
	{
		return false;
	}

	bool bRet = false;

	auto maxPos = vecBuffer.size() - 10;
	
	char *lpData = &vecBuffer.front();
	for (UINT uIndex = 0; uIndex < maxPos; uIndex++)
	{
		if (findFlacTag(lpData+uIndex, __TitleField, sizeof(__TitleField) - 1, m_MediaTag.strTitle))
		{
			bRet = true;
			break;
		}
	}

	for (UINT uIndex = 0; uIndex < maxPos; uIndex++)
	{
		if (findFlacTag(lpData+uIndex, __ArtistField, sizeof(__ArtistField) - 1, m_MediaTag.strArtist))
		{
			bRet = true;
			break;
		}
	}

	for (UINT uIndex = 0; uIndex < maxPos; uIndex++)
	{
		if (findFlacTag(lpData+uIndex, __AlbumField, sizeof(__AlbumField)-1, m_MediaTag.strAlbum))
		{
			bRet = true;
			break;
		}
	}

	return bRet;
}

void CMediaRes::onAsyncTask()
{
	__Ensure(g_pRootMediaRes);
	(void)g_pRootMediaRes->m_MediaObserver.findRelatedMedia(*this, E_MediaSetType::MST_Playlist);

	(void)g_pRootMediaRes->m_MediaObserver.findRelatedMedia(*this, E_MediaSetType::MST_Album);

	if (E_MediaFileType::MFT_MP3 == m_eFileType || E_MediaFileType::MFT_FLAC == m_eFileType)
	{
		(void)ReadTag();
	}
}

bool CMediaRes::CheckRelatedMediaSetChanged(const tagMediaSetChanged& MediaSetChanged)
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

void CMediaRes::Clear()
{
	m_mapSubCueFile.clear();
	m_alSubCueFile.clear();
    CPathObject::Clear();
}

CMediaRes* CMediaRes::GetParent() const
{
	if (NULL != m_pParentDir)
	{
		return (CMediaRes*)m_pParentDir;
	}
	
	return g_pRootMediaRes;
}
