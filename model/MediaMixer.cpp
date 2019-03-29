
#include <model.h>

void CMediaMixer::clear()
{
	m_lstMedias.clear();
	
	m_mapData.clear();

	for (auto& pr : m_mapSortData)
	{
		pr.second.clear();
	}
}

void CMediaMixer::_remove(const CMediaMixture& MediaMixture)
{
	for (auto pMedia : { (CMedia*)MediaMixture.GetPlayItem(), (CMedia*)MediaMixture.GetAlbumItem() })
	{
		if (NULL != pMedia)
		{
			m_lstMedias.del(pMedia);
		}
	}

	for (auto& pr : m_mapSortData)
	{
		pr.second.del(&MediaMixture);
	}
}

void CMediaMixer::_erase(SSet<CMedia*>& setMedias, const function<void(CMediaMixture&)>& cb)
{
    m_mapData.del_ex([&](pair<wstring const, CMediaMixture>& pr) {
		CMediaMixture& MediaMixture = pr.second;

		bool bFlag = false;
		for (auto pMedia : { (CMedia*)MediaMixture.GetPlayItem(), (CMedia*)MediaMixture.GetAlbumItem() })
		{
			if (NULL != pMedia)
			{
				if (0 != setMedias.del(pMedia))
				{
					bFlag = true;
					break;
				}
			}
		}

		if (bFlag)
		{
			cb(MediaMixture);

			if (!setMedias)
			{
				return E_DelConfirm::DC_YesAbort;
			}
			else
			{
				return E_DelConfirm::DC_Yes;
			}
		}
		else
		{
			return E_DelConfirm::DC_No;
		}
	});
}

void CMediaMixer::remove(const TD_MediaList& lstMedias)
{
	SSet<CMedia*> setMedias(lstMedias);
	_erase(setMedias, [&](CMediaMixture& MediaMixture){
		_remove(MediaMixture);
	});
}

void CMediaMixer::update(const TD_MediaList& lstMedias)
{
    SList<pair<CMediaMixture*, CMediaMixture>> lstUpdate;

    SSet<CMedia*> setMedias(lstMedias);
    _erase(setMedias, [&](CMediaMixture& MediaMixture) {
        lstUpdate.add({ &MediaMixture, MediaMixture });
    });

    SMap<CMediaMixture*, CMediaMixture*> mapUpdate;
    for (auto& pr : lstUpdate)
    {
        auto& MediaMixture = m_mapData.insert(pr.second.GetPath(), pr.second);
        MediaMixture.UpdateTitle();

        mapUpdate.set(pr.first, &MediaMixture);
    }

    for (auto& pr : m_mapSortData)
    {
        SMap<CMediaMixture*, CMediaMixture*> t_mapUpdate(mapUpdate);

        auto& vecData = pr.second;
        vecData([&](CMediaMixture& MediaMixture, size_t pos) {
            if (t_mapUpdate.del(&MediaMixture, [&](pair<CMediaMixture* const, CMediaMixture*>& pr) {
                vecData.set(pos, pr.second);
            }))
            {
                if (!t_mapUpdate)
                {
                    return false;
                }
            }

            return true;
        });

        if (E_SortMediaType::SMT_Default != pr.first)
        {
            _sort(vecData, pr.first);
        }
    }
}

CMediaMixer& CMediaMixer::add(CMedia& media)
{
    wstring strPath = media.GetPath();

    bool bFlag = false;
    m_mapData.get(strPath, [&](CMediaMixture& MediaMixture) {
        if (MediaMixture.Add(media))
        {
            bFlag = true;
            return false;
        }

        return true;
    });
    if (!bFlag)
    {
        m_vecData.add(m_mapData.insert(strPath, CMediaMixture(media)));
    }

    m_lstMedias.add(media);

    return *this;
}

const TD_MediaMixtureVector& CMediaMixer::add(const TD_MediaList& lstMedias)
{
    lstMedias([&](CMedia& Media) {
        add(Media);
    });

    return m_vecData;
}

const TD_MediaMixtureVector& CMediaMixer::add(CMediaSet& mediaSet)
{
    TD_MediaList lstMedias;
    mediaSet.GetMedias(lstMedias);
    return add(lstMedias);
}

const TD_MediaMixtureVector& CMediaMixer::get(E_SortMediaType eSortType)
{
    if (E_SortMediaType::SMT_Default == eSortType)
    {
        return m_vecData;
    }

    TD_MediaMixtureVector *pvecData = NULL;
    m_mapSortData.get(eSortType, [&](TD_MediaMixtureVector& vecData) {
        if (vecData.size() != m_vecData.size())
        {
            vecData.assign(m_vecData);
            _sort(vecData, eSortType);
        }

        pvecData = &vecData;
    });
    if (NULL != pvecData)
    {
        return *pvecData;
    }

    auto& vecData = m_mapSortData.insert(eSortType, m_vecData);
    _sort(vecData, eSortType);
    return vecData;
}

void CMediaMixer::_sort(TD_MediaMixtureVector& vecData, E_SortMediaType eSortType)
{
    vecData.qsort([&](const CMediaMixture& lhs, const CMediaMixture& rhs) {
        return _compare(eSortType, lhs, rhs);
    });
}

bool CMediaMixer::_compare(E_SortMediaType eSortType, const CMediaMixture& lhs, const CMediaMixture& rhs)
{
    switch (eSortType)
    {
    case E_SortMediaType::SMT_ByPath:
    {
        int iRet = util::StrCompareUseCNCollate(lhs.GetDir(), rhs.GetDir());
        if (iRet < 0)
        {
            return true;
        }
        else if (0 == iRet)
        {
            return util::StrCompareUseCNCollate(lhs.GetTitle(), rhs.GetTitle()) < 0;
        }
    }

    break;
    case E_SortMediaType::SMT_ByTitle:
        return util::StrCompareUseCNCollate(lhs.GetTitle(), rhs.GetTitle()) < 0;

        break;
    case E_SortMediaType::SMT_ByFileType:
        return lhs.GetFileType() < rhs.GetFileType();

        break;
    case E_SortMediaType::SMT_ByAddTime:
        return lhs.GetMediaTime() < rhs.GetMediaTime();

        break;
    case E_SortMediaType::SMT_ByPlaylist:
        if (!lhs.GetPlaylistName().empty())
        {
            if (rhs.GetPlaylistName().empty())
            {
                return true;
            }

            return util::StrCompareUseCNCollate(lhs.GetPlaylistName(), rhs.GetPlaylistName()) < 0;
        }

        break;
    case E_SortMediaType::SMT_BySinger:
        if (!lhs.GetSingerName().empty())
        {
            if (rhs.GetSingerName().empty())
            {
                return true;
            }

            int iRet = util::StrCompareUseCNCollate(lhs.GetSingerName(), rhs.GetSingerName());
            if (iRet < 0)
            {
                return true;
            }
            else if (0 == iRet)
            {
                return util::StrCompareUseCNCollate(lhs.GetAlbumName(), rhs.GetAlbumName()) < 0;
            }
        }

        break;
    case E_SortMediaType::SMT_ByCustom:
        if (m_cbSort)
        {
            return m_cbSort(lhs, rhs);
        }

        break;
    }

    return false;
}

bool CMediaMixer::getMediaMixture(const function<void(const CMediaMixture&)>& cb, UINT uIndex, E_SortMediaType eSortType)
{
    auto& vecData = get(eSortType);
    return vecData.get(uIndex, [&](CMediaMixture& MediaMixture) {
        cb(MediaMixture);
    });
}

bool CMediaMixer::getMedia(const function<void(CMedia&)>& cb, UINT uIndex, E_SortMediaType eSortType)
{
    auto& vecData = get(eSortType);

    CMedia *pMedia = NULL;
    if (!vecData.get(uIndex, [&](CMediaMixture& MediaMixture) {
        pMedia = MediaMixture.GetMedia();
    }))
    {
        return false;
    }

    if (NULL == pMedia)
    {
        return false;
    }

    cb(*pMedia);

    return true;
}
