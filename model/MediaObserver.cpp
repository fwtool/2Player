
#include <model.h>

#include "MediaObserver.h"

#include <Player.h>

void CMediaObserver::renameMedia(IMedia& media, const wstring& strNewName)
{
	m_ModelObserver.renameMedia(media, strNewName);
}

CMedia *CMediaObserver::findRelatedMedia(IMedia& media, E_MediaSetType eMediaSetType)
{
	media.ClearRelatedMediaSet(eMediaSetType);

	if (E_MediaSetType::MST_Album == eMediaSetType)
	{
		media.ClearRelatedMediaSet(E_MediaSetType::MST_Singer);
	}
	
	wstring strPath = media.GetPath();
	
	tagFindMediaPara FindPara(E_FindMediaMode::FMM_MatchPath, strPath, eMediaSetType, true);
	FindPara.strDir = fsutil::GetParentDir(strPath);

	tagFindMediaResult FindResult;
	m_model.getMediaLib().FindMedia(FindPara, FindResult);
	if (FindResult.lstRetMedias)
	{
		CMedia *pRelatedMedia = NULL;
		FindResult.lstRetMedias.getFront([&](CMedia& RelatedMedia) {
			media.SetRelatedMediaSet(RelatedMedia);

			if (E_MediaSetType::MST_Album == RelatedMedia.GetMediaSetType())
			{
				CSinger *pSinger = ((CAlbumItem&)RelatedMedia).GetSinger();
				if (NULL != pSinger)
				{
					media.SetRelatedMediaSet(*pSinger);
				}
			}

			pRelatedMedia = &RelatedMedia;
		});

		return pRelatedMedia;
	}
	else
	{
		if (NULL != FindResult.pRetSinger)
		{
			media.SetRelatedMediaSet(*FindResult.pRetSinger);
		}

		return NULL;
	}
}

int CMediaObserver::checkMedia(const wstring& strAbsPath)
{
	return CPlayer::CheckMedia(strAbsPath);
}

UINT CMediaObserver::getSingerImgPos(UINT uSingerID)
{
	return m_ModelObserver.GetSingerImgPos(uSingerID);
}
