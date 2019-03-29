
#include <model.h>

#include "MediaObserver.h"

#include "Player.h"

#include "SimilarCheck.h"

CModel::CModel(IModelObserver& ModelObserver)
	: m_ModelObserver(ModelObserver)

	, m_db("data.db")
	, m_dao(m_db)
	
	, m_DataMgr(m_dao)
	, m_BackupMgr(m_dao)

	, m_RootMediaRes(CMediaObserver::createInst(*this, ModelObserver))

	, m_MediaLib(L"媒体库")

	, m_PlaylistMgr(m_MediaLib, m_dao)

	, m_PlayMgr(m_PlaylistMgr, m_DataMgr, ModelObserver)

	, m_SingerMgr(m_MediaLib, m_dao)
{
}

bool CModel::init()
{
    __EnsureReturn(m_db.Connect(), false);

	__EnsureReturn(m_DataMgr.init(), false);

	if (!m_DataMgr.getOption().strRootDir.empty())
	{
        __EnsureReturn(initRootMediaRes(), false);

		__EnsureReturn(m_PlaylistMgr.Init(), false);

		__EnsureReturn(m_SingerMgr.Init(), false);
	
		__EnsureReturn(m_PlayMgr.init([&]() {
			m_ModelObserver.onPlayFinish();
		}), false);
	}

    m_BackupMgr.init();

	return true;
}

bool CModel::initRootMediaRes(const wstring& strRootDir)
{
	if (!strRootDir.empty())
	{
		__EnsureReturn(m_dao.SetOption(OI_RootDir, strRootDir), false);

		m_DataMgr.getOption().strRootDir = strRootDir;
	}

	m_RootMediaRes.init(m_DataMgr.getOption().strRootDir, m_DataMgr.getOption().vecAttachDir);

	m_ModelObserver.refreshMediaRes();

	return true;
}

void CModel::attachDir(const wstring& strDir)
{
	auto& vecAttachDir = m_DataMgr.getOption().vecAttachDir;
	for (auto itr = vecAttachDir.begin(); itr != vecAttachDir.end();)
	{
		if (fsutil::CheckSubPath(strDir, *itr))
		{
			itr = vecAttachDir.erase(itr);
		}
		else
		{
			++itr;
		}
	}
	vecAttachDir.push_back(strDir);

	m_RootMediaRes.init(m_DataMgr.getOption().strRootDir, vecAttachDir);
	m_ModelObserver.refreshMediaRes();
}

void CModel::detachDir(const wstring& strDir)
{
	auto& vecAttachDir = m_DataMgr.getOption().vecAttachDir;
	for (auto itr = vecAttachDir.begin(); itr != vecAttachDir.end(); ++itr)
	{
		if (util::StrMatchIgnoreCase(strDir, *itr))
		{
			(void)vecAttachDir.erase(itr);

			m_RootMediaRes.init(m_DataMgr.getOption().strRootDir, vecAttachDir);
			m_ModelObserver.refreshMediaRes();

			return;
		}
	}
}

bool CModel::renameMedia(const wstring& strOldOppPath, const wstring& strNewOppPath, bool bDir)
{
	map<CMedia*, wstring> mapUpdateMedias;
	if (bDir)
	{
		tagFindMediaResult FindResult;
		tagFindMediaPara FindPara(E_FindMediaMode::FMM_RenameDir);
		FindPara.strDir = strOldOppPath;
		FindPara.strRenameDir = strNewOppPath;

		m_MediaLib.FindMedia(FindPara, FindResult);

		if (!FindResult.mapSingerDirChanged.empty())
		{
			__EnsureReturn(m_dao.updateSingerPath(FindResult.mapSingerDirChanged), false);
		}

		FindResult.lstRetMedias([&](CMedia& Media) {
			mapUpdateMedias[&Media] = strNewOppPath + Media.GetPath().substr(strOldOppPath.size());
		});
	}
	else
	{
		tagFindMediaResult FindResult;
		tagFindMediaPara FindPara(E_FindMediaMode::FMM_MatchPath, strOldOppPath);
		FindPara.strDir = fsutil::GetParentDir(strOldOppPath);

		m_MediaLib.FindMedia(FindPara, FindResult);

		FindResult.lstRetMedias([&](CMedia& Media) {
			mapUpdateMedias[&Media] = strNewOppPath;
		});
	}

	if (!mapUpdateMedias.empty())
	{
		__EnsureReturn(this->updateMediaPath(mapUpdateMedias), false);
	}

	if (bDir)
	{
		__EnsureReturn(this->updateDir(strOldOppPath, strNewOppPath), false);
	}
	else
	{
		__EnsureReturn(this->updateFile({ { strOldOppPath, strNewOppPath } }), false);
	}
	
	return true;
}

bool CModel::removeMedia(const TD_MediaList& lstMedias)
{
	TD_PlayItemList lstPlayItems;
	TD_AlbumItemList lstAlbumItems;
	lstMedias([&](CMedia& Media) {
		if (E_MediaSetType::MST_Playlist == Media.GetMediaSetType())
		{
			lstPlayItems.add(&Media);
		}
		else if (E_MediaSetType::MST_Album == Media.GetMediaSetType())
		{
			lstAlbumItems.add(&Media);
		}
	});

	tagUpdateMedia UpdateMedia;
	UpdateMedia.lstPlayItems = lstPlayItems;
	UpdateMedia.lstAlbumItems = lstAlbumItems;
	UpdateMedia.bRemove = true;
	m_ModelObserver.updateMedia(UpdateMedia);

	if (lstPlayItems)
	{
		__EnsureReturn(m_PlaylistMgr.RemovePlayItems(lstPlayItems), false);
	}

	if (lstAlbumItems)
	{
		__EnsureReturn(m_SingerMgr.RemoveAlbumItems(lstAlbumItems), false);
	}

	return true;
}

bool CModel::moveFiles(const wstring& strDir, const SMap<wstring, wstring>& mapUpdateFiles)
{
	tagFindMediaPara FindPara(E_FindMediaMode::FMM_MatchFiles);
	for (auto& pr : mapUpdateFiles)
	{
		FindPara.setFiles.insert(pr.first);
	}

	map<CMedia*, wstring> mapUpdateMedias;
	
	tagFindMediaResult FindResult;
	m_MediaLib.FindMedia(FindPara, FindResult);

	FindResult.lstRetMedias([&](CMedia& Media) {
		mapUpdateMedias[&Media] = strDir + fsutil::backSlant + Media.GetName();
	});
	if (!mapUpdateMedias.empty())
	{
		__EnsureReturn(this->updateMediaPath(mapUpdateMedias), false);
	}

	__EnsureReturn(this->updateFile(mapUpdateFiles), false);

	return true;
}

bool CModel::removeFiles(const std::set<wstring>& setFiles)
{
	tagFindMediaPara FindPara(E_FindMediaMode::FMM_MatchFiles);
	FindPara.setFiles = setFiles;

	tagFindMediaResult FindResult;
	m_MediaLib.FindMedia(FindPara, FindResult);
	if (FindResult.lstRetMedias)
	{
		__EnsureReturn(this->removeMedia(FindResult.lstRetMedias), false);
	}

	SHashMap<wstring, TD_PlayItemList> mapPlayItem;
	for (auto& PlayItem : m_PlaylistMgr.GetPlayinglist().m_lstPlayItems)
	{
		mapPlayItem.insert(util::StrLowerCase(PlayItem.GetPath())).add(&PlayItem);
	}

	TD_PlayItemList arrDelPlayingItem;
	for (auto& strFile : setFiles)
	{
        mapPlayItem.del(util::StrLowerCase(strFile), [&](pair<wstring const, TD_PlayItemList>& pr) {
			arrDelPlayingItem.add(pr.second);
		});
	}

	if (arrDelPlayingItem)
	{
		tagUpdateMedia UpdateMedia;
		UpdateMedia.lstPlayItems = arrDelPlayingItem;
		UpdateMedia.bRemove = true;
		m_ModelObserver.updateMedia(UpdateMedia);

		__EnsureReturn(m_PlayMgr.remove(arrDelPlayingItem), false);
	}

	__EnsureReturn(m_dao.deleteFiles(setFiles), false);

	__EnsureReturn(m_DataMgr.initAlarmmedia(), false);

	return true;
}

bool CModel::updateMediaPath(const map<CMedia*, wstring>& mapUpdateMedias)
{
	wstring strBaseDir;
	wstring strOppPath;

	TD_PlayItemList lstPlayItems;
	TD_AlbumItemList lstAlbumItems;

	CDaoTransGuard transGuard(m_dao);

	for (auto pr : mapUpdateMedias)
	{
		CMedia *pMedia = pr.first;

		strOppPath = pr.second;

		if (E_MediaSetType::MST_Playlist == pMedia->GetMediaSetType())
		{
			lstPlayItems.add(pMedia);
			__EnsureReturn(m_dao.updatePlayItemPath(pMedia->m_uID, strOppPath), false);
		}
		else if (E_MediaSetType::MST_Album == pMedia->GetMediaSetType())
		{
			lstAlbumItems.add(pMedia);

			strBaseDir = pMedia->GetBaseDir();
			strOppPath = fsutil::GetOppPath(strOppPath, strBaseDir);
			__EnsureContinue(!strOppPath.empty());

			__EnsureReturn(m_dao.updateAlbumItemPath(pMedia->m_uID, strOppPath), false);
		}

		pMedia->UpdatePath(strOppPath);
	}
	
	if (lstPlayItems || lstAlbumItems)
	{
		tagUpdateMedia UpdateMedia;
		UpdateMedia.lstPlayItems = lstPlayItems;
		UpdateMedia.lstAlbumItems = lstAlbumItems;
		m_ModelObserver.updateMedia(UpdateMedia);
	}
	
	return true;
}

bool CModel::updateFile(const map<wstring, wstring>& mapUpdateFiles)
{
	map<CMedia*, wstring> mapUpdateMedias;
	for (auto& PlayItem : m_PlaylistMgr.GetPlayinglist().m_lstPlayItems)
	{
		auto itr = mapUpdateFiles.find(PlayItem.GetPath());
		if (itr != mapUpdateFiles.end())
		{
			mapUpdateMedias[&PlayItem] = itr->second;
		}
	}

	CDaoTransGuard transGuard(m_dao);

	if (!mapUpdateMedias.empty())
	{
		__EnsureReturn(this->updateMediaPath(mapUpdateMedias), false);

		m_ModelObserver.refreshPlayingList();
	}

	for (auto& pr : mapUpdateFiles)
	{
		__EnsureReturn(m_dao.updateFiles(pr.first, pr.second, false), false);
	}

	__EnsureReturn(m_DataMgr.initAlarmmedia(), false);

	return true;
}

bool CModel::updateDir(const wstring& strOldPath, const wstring& strNewPath)
{
	map<CMedia*, wstring> mapUpdateMedias;
	for (auto& PlayItem : m_PlaylistMgr.GetPlayinglist().m_lstPlayItems)
	{
		if (fsutil::CheckSubPath(strOldPath, PlayItem.GetPath()))
		{
			mapUpdateMedias[&PlayItem] = strNewPath + PlayItem.GetPath().substr(strOldPath.size());
		}
	}

	CDaoTransGuard transGuard(m_dao);

	if (!mapUpdateMedias.empty())
	{
		__EnsureReturn(this->updateMediaPath(mapUpdateMedias), false);

		m_ModelObserver.refreshPlayingList();
	}

	__EnsureReturn(m_dao.updateFiles(strOldPath, strNewPath, true), false);

	__EnsureReturn(m_DataMgr.initAlarmmedia(), false);

	return true;
}

UINT CModel::exportMedia(const tagExportOption& ExportOption, const CB_exportorMedia& cb)
{
	UINT uCount = 0;

	const TD_IMediaMap& mapMedias = ExportOption.mapMedias;

	wstring strSrcFile, strSnkFile, strDstDir;
	for (auto& pr : mapMedias)
	{
		strDstDir = pr.first;

		if (!fsutil_win::CreateDir(strDstDir))
		{
			continue;
		}

		strDstDir += fsutil::backSlant;

        //int nBaseIndex = (int)pow(10.0, ceil(log10((double)pr.second.size())));

		bool bCancel = false;
		pr.second([&](IMedia& media) {
			strSrcFile = media.GetAbsPath();
			strSnkFile = strDstDir + (ExportOption.bActualMode ? media.GetName() : media.GetExportFileName());

			int iSrcSize = fsutil::GetFileSize(strSrcFile);
			if (iSrcSize > 0)
			{
				if (!ExportOption.bCompareFile
					|| fsutil::GetFileSize(strSnkFile) != iSrcSize
					|| fsutil::GetFileModifyTime(strSrcFile) != fsutil::GetFileModifyTime(strSrcFile))
				{
                    if (fsutil_win::copyFile(util::WStrToStr(strSrcFile), util::WStrToStr(strSnkFile), true))
					{
						uCount++;
					}
				}
			}

			if (!cb(1, strSnkFile))
			{
				bCancel = true;
				return false;
			}

			return true;
		});

		if (bCancel)
		{
			break;
		}
	}

	return uCount;
}

void CModel::checkDuplicateMedia(E_CheckDuplicateMode eMode, const TD_MediaList& lstMedias, CB_checkDuplicateMedia cb, SArray<TD_MediaList>& arrResult)
{
    SArray<pair<wstring, CMedia*>> arrMedias = lstMedias.map([&](CMedia& media) {
		return std::make_pair(util::StrLowerCase(media.GetTitle()), &media);
	});
	
	wstring strDir1;

	SSet<CMedia*> setDuplicateMedia;
	
    arrMedias([&](pair<wstring, CMedia*>& pr, size_t pos) {
		CMedia *pMedia1 = pr.second;
		if (!cb(*pMedia1))
		{
			return false;
		}

		if (setDuplicateMedia.includes(pMedia1))
		{
			return true;
		}

		const wstring& strTitle1 = pr.first;

		if (E_CheckDuplicateMode::CDM_SamePath == eMode)
		{
			strDir1 = util::StrLowerCase(pMedia1->GetDir());
		}

		TD_MediaList lstDuplicateMedia;

        arrMedias(pos + 1, [&](pair<wstring, CMedia*>& pr) {
			CMedia& Media2 = *pr.second;

			if (E_CheckDuplicateMode::CDM_SamePath == eMode)
			{
				if (Media2.GetFileType() != pMedia1->GetFileType())
				{
					return;
				}

				if (pr.first != strTitle1)
				{
					return;
				}

				if (util::StrLowerCase(Media2.GetDir()) != strDir1)
				{
					return;
				}
			}
			else if (E_CheckDuplicateMode::CDM_SameName == eMode)
			{
				if (Media2.GetFileType() != pMedia1->GetFileType())
				{
					return;
				}

				if (pr.first != strTitle1)
				{
					return;
				}
			}
			else if (E_CheckDuplicateMode::CDM_SameTitle == eMode)
			{
				if (pr.first != strTitle1)
				{
					return;
				}
			}

			lstDuplicateMedia.add(Media2);
			setDuplicateMedia.add(&Media2);
		});

		if (lstDuplicateMedia)
		{
			lstDuplicateMedia.addFront(pMedia1);
			arrResult.add(lstDuplicateMedia);
		}

		return true;
    });
}

void CModel::checkSimilarFile(TD_MediaResList& lstMediaRes, CB_checkSimilarFile cb, TD_SimilarFile& arrResult)
{
	PairList<CMediaRes*, tagMediaResInfo> plMediaResInfo;
	lstMediaRes([&](CMediaRes& MediaRes) {
		plMediaResInfo.addPair( &MediaRes, tagMediaResInfo(MediaRes.GetPath(), MediaRes.GetFileSizeString()) );
	});

	CSimilarCheck::checkSimilarFile(plMediaResInfo, cb, arrResult);
}

void CModel::checkSimilarFile(TD_MediaResList& lstMediaRes1, TD_MediaResList& lstMediaRes2, CB_checkSimilarFile cb, TD_SimilarFile& arrResult)
{
	PairList<CMediaRes*, tagMediaResInfo> plMediaResInfo1;
	lstMediaRes1([&](CMediaRes& MediaRes) {
		plMediaResInfo1.addPair(&MediaRes, tagMediaResInfo(MediaRes.GetPath(), MediaRes.GetFileSizeString()));
	});

	PairList<CMediaRes*, tagMediaResInfo> plMediaResInfo2;
	lstMediaRes2([&](CMediaRes& MediaRes) {
		plMediaResInfo2.addPair(&MediaRes, tagMediaResInfo(MediaRes.GetPath(), MediaRes.GetFileSizeString()));
	});

	CSimilarCheck::checkSimilarFile(plMediaResInfo1, plMediaResInfo2, cb, arrResult);
}

bool CModel::clearData()
{
	__EnsureReturn(m_DataMgr.clearData(), false);

	m_PlaylistMgr.GetPlayinglist().m_lstPlayItems.clear();
	m_PlaylistMgr.GetPlaylists().clear();

	m_PlayMgr.clear();

	m_SingerMgr.m_lstGroups.clear();
	m_SingerMgr.m_lstRootSingers.clear();

	return true;
}

void CModel::close()
{
	m_PlayMgr.getPlayer().Stop();

	m_BackupMgr.close();

	m_DataMgr.getOption().uPlayingItem = m_PlayMgr.m_uPlayingItem;
	m_DataMgr.saveOption();

	(void)m_db.Disconnect();
}

bool CModel::restoreDB(const wstring& strFile)
{
	__EnsureReturn(clearData(), false);

	close();

	__EnsureReturn(CopyFile((m_BackupMgr.getBackupDir() + strFile).c_str(), L"data.db", FALSE), false);

	__EnsureReturn(init(), false);

	return true;
}
