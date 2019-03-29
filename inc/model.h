
#pragma once

#ifdef __ModelPrj
#define __ModelExt __declspec(dllexport)
#else
#define __ModelExt __declspec(dllimport)
#endif

#include "MediaDef.h"

#include "MediaMixer.h"

#include "Dao.h"

#include "PlaylistMgr.h"

#include <Player.h>

#include "PlayMgr.h"

#include "SingerMgr.h"

#include "DataMgr.h"

#include "BackupMgr.h"

#include "AutoMatch.h"

struct tagUpdateMedia
{
	TD_PlayItemList lstPlayItems;
	TD_AlbumItemList lstAlbumItems;
	bool bRemove = false;
};

interface IModelObserver
{
	virtual void refreshMediaRes() = 0;
	
	virtual void refreshPlayingList() = 0;

	virtual void onPlay(UINT uPlayingItem, UINT uPrevPlayingItem) = 0;
	virtual void onPlayFinish() = 0;
	
	virtual UINT GetSingerImgPos(UINT uSingerID) = 0;

	virtual void renameMedia(IMedia& media, const wstring& strNewName) = 0;

	virtual void updateMedia(const tagUpdateMedia& UpdateMedia) = 0;
};

enum class E_CheckDuplicateMode
{
	CDM_SamePath
	, CDM_SameName
	, CDM_SameTitle
};

struct tagExportOption
{
	bool bActualMode = false;
	bool bCompareFile = false;
	bool bDeleteOther = true;

	wstring strExportPath;

	TD_IMediaMap mapMedias;
};

using CB_checkDuplicateMedia = const function<bool(CMedia&)>&;

using CB_checkSimilarFile = const function<bool(CMediaRes&)>&;

using TD_SimilarFileGroup = SArray<pair<CMediaRes*, UINT>>;

using TD_SimilarFile = SArray<TD_SimilarFileGroup>;

interface IModel
{
	virtual CRootMediaRes& getRootMediaRes() = 0;
	virtual CMediaSet& getMediaLib() = 0;
	
	virtual CDataMgr& getDataMgr() = 0;
	virtual CBackupMgr& getBackupMgr() = 0;

	virtual CPlaylistMgr& getPlaylistMgr() = 0;
	virtual CPlayMgr& getPlayMgr() = 0;
	
	virtual CSingerMgr& getSingerMgr() = 0;

	virtual bool init() = 0;

	virtual bool initRootMediaRes(const wstring& strRootDir = L"") = 0;
	
	virtual void attachDir(const wstring& strDir) = 0;
	virtual void detachDir(const wstring& strDir) = 0;

	virtual bool renameMedia(const wstring& strOldOppPath, const wstring& strNewOppPath, bool bDir) = 0;

	virtual bool removeMedia(const TD_MediaList& lstMedias) = 0;

	virtual bool moveFiles(const wstring& strDir, const SMap<wstring, wstring>& mapUpdateFiles) = 0;

	virtual bool removeFiles(const std::set<wstring>& setFiles) = 0;

	virtual bool updateMediaPath(const map<CMedia*, wstring>& mapUpdateMedias) = 0;

	virtual bool updateFile(const map<wstring, wstring>& mapUpdateFiles) = 0;
	virtual bool updateDir(const wstring& strOldPath, const wstring& strNewPath) = 0;

	using CB_exportorMedia = function<bool(UINT uProgressOffset, const wstring& strSnkFile)>;
	virtual UINT exportMedia(const tagExportOption& ExportOption, const CB_exportorMedia& cb) = 0;

	virtual void checkDuplicateMedia(E_CheckDuplicateMode eMode, const TD_MediaList& lstMedias
		, CB_checkDuplicateMedia cb, SArray<TD_MediaList>& arrResult) = 0;

	virtual void checkSimilarFile(TD_MediaResList& lstMediaRes, CB_checkSimilarFile cb, TD_SimilarFile& arrResult) = 0;
	virtual void checkSimilarFile(TD_MediaResList& lstMediaRes1, TD_MediaResList& lstMediaRes2, CB_checkSimilarFile cb, TD_SimilarFile& arrResult) = 0;

	virtual bool clearData() = 0;

	virtual void close() = 0;

	virtual bool restoreDB(const wstring& strFile) = 0;
};

class __ModelExt CModel : public IModel
{
public:
	CModel(IModelObserver& ModelObserver);

private:
	IModelObserver& m_ModelObserver;

	CSQLiteDB m_db;
	CDao m_dao;

	CDataMgr m_DataMgr;

	CBackupMgr m_BackupMgr;

	CRootMediaRes m_RootMediaRes;

	CMediaSet m_MediaLib;

	CPlaylistMgr m_PlaylistMgr;

	CPlayMgr m_PlayMgr;

	CSingerMgr m_SingerMgr;
	
public:
	CDataMgr& getDataMgr() override
	{
		return m_DataMgr;
	}

	CBackupMgr& getBackupMgr() override
	{
		return m_BackupMgr;
	}

	CRootMediaRes& getRootMediaRes() override
	{
		return m_RootMediaRes;
	}

	CMediaSet& getMediaLib() override
	{
		return m_MediaLib;
	}

	CPlaylistMgr& getPlaylistMgr() override
	{
		return m_PlaylistMgr;
	}

	CPlayMgr& getPlayMgr() override
	{
		return m_PlayMgr;
	}

	CSingerMgr& getSingerMgr() override
	{
		return m_SingerMgr;
	}

	bool init() override;

	bool initRootMediaRes(const wstring& strRootDir = L"") override;

	void attachDir(const wstring& strDir) override;
	void detachDir(const wstring& strDir) override;

	bool renameMedia(const wstring& strOldOppPath, const wstring& strNewOppPath, bool bDir) override;

	bool removeMedia(const TD_MediaList& lstMedias) override;

	bool moveFiles(const wstring& strDir, const SMap<wstring, wstring>& mapUpdateFiles) override;

	bool removeFiles(const std::set<wstring>& setFiles) override;

	bool updateMediaPath(const map<CMedia*, wstring>& mapUpdateMedias) override;
	
	bool updateFile(const map<wstring, wstring>& mapUpdateFiles) override;
	bool updateDir(const wstring& strOldPath, const wstring& strNewPath) override;

	UINT exportMedia(const tagExportOption& ExportOption, const CB_exportorMedia& cb) override;

	void checkDuplicateMedia(E_CheckDuplicateMode eMode, const TD_MediaList& lstMedias
		, CB_checkDuplicateMedia cb, SArray<TD_MediaList>& arrResult) override;

	void checkSimilarFile(TD_MediaResList& lstMediaRes, CB_checkSimilarFile cb, TD_SimilarFile& arrResult) override;
	void checkSimilarFile(TD_MediaResList& lstMediaRes1, TD_MediaResList& lstMediaRes2, CB_checkSimilarFile cb, TD_SimilarFile& arrResult) override;

	bool clearData() override;

	void close() override;

	bool restoreDB(const wstring& strFile) override;
};
