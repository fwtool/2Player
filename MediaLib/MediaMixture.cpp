
#include <MediaDef.h>

bool CMediaMixture::Add(CMedia& media)
{	
	if (E_MediaSetType::MST_Playlist == media.GetMediaSetType())
	{
		if (NULL != m_pPlayItem)
		{
			return false;
		}

		m_pPlayItem = (CPlayItem*)&media;
		m_strPlaylistName = m_pPlayItem->GetPlaylistName();
	}
	else if (E_MediaSetType::MST_Album == media.GetMediaSetType())
	{
		if (NULL != m_pAlbumItem)
		{
			return false;
		}

		m_pAlbumItem = (CAlbumItem*)&media;
		m_strAlbumName = m_pAlbumItem->GetAlbumName();
		m_strSingerName = m_pAlbumItem->GetSingerName();
	}

	if (0 == m_MediaTime.m_time || media.GetAddTime() < m_MediaTime)
	{
		m_MediaTime = media.GetAddTime();
	}

	return true;
}

void CMediaMixture::UpdateTitle()
{
	if (NULL != m_pPlayItem)
	{
		m_strTitle = m_pPlayItem->GetTitle();
	}
	else if (NULL != m_pAlbumItem)
	{
		m_strTitle = m_pAlbumItem->GetTitle();
	}
}

bool CMediaMixture::Remove(CMedia *pMedia)
{
	if (pMedia == m_pPlayItem)
	{
		m_pPlayItem = NULL;
		m_strPlaylistName.clear();

		if (NULL != m_pAlbumItem)
		{
			m_MediaTime = m_pAlbumItem->GetAddTime();
		}

		return true;
	}
	else if (pMedia == m_pAlbumItem)
	{
		m_pAlbumItem = NULL;
		m_strAlbumName.clear();
		m_strSingerName.clear();

		if (NULL != m_pPlayItem)
		{
			m_MediaTime = m_pPlayItem->GetAddTime();
		}

		return true;
	}

	return false;
}

CAlbum* CMediaMixture::GetAlbum() const
{
	if (NULL != m_pAlbumItem)
	{
		return (CAlbum*)m_pAlbumItem->m_pParent;
	}

	return NULL;
}

CSinger* CMediaMixture::GetSinger() const
{
	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem->GetSinger();
	}

	return NULL;
}

CMedia* CMediaMixture::GetMedia() const
{
	if (NULL != m_pPlayItem)
	{
		return m_pPlayItem;
	}

	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem;
	}

	return NULL;
}

wstring CMediaMixture::GetDir() const
{
	if (NULL != m_pPlayItem)
	{
		return m_pPlayItem->GetDir();
	}
	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem->GetDir();
	}

	return L"";
}

wstring CMediaMixture::GetPath() const
{
	if (NULL != m_pPlayItem)
	{
		return m_pPlayItem->GetPath();
	}
	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem->GetPath();
	}

	return L"";
}

E_MediaFileType CMediaMixture::GetFileType() const
{
	if (NULL != m_pPlayItem)
	{
		return m_pPlayItem->GetFileType();
	}
	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem->GetFileType();
	}

	return E_MediaFileType::MFT_Null;
}

wstring CMediaMixture::GetFileTypeString() const
{
	if (NULL != m_pPlayItem)
	{
		return m_pPlayItem->GetFileTypeString();
	}
	if (NULL != m_pAlbumItem)
	{
		return m_pAlbumItem->GetFileTypeString();
	}

	return L"";
}

wstring CMediaMixture::GetSingerAlbumString() const
{
	wstring strSingerAlbum = m_strSingerName;
	if (!strSingerAlbum.empty())
	{
		strSingerAlbum.append(__CNDotDelimiter).append(m_strAlbumName);
	}

	return strSingerAlbum;
}
