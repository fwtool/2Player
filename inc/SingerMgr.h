#pragma once

class __ModelExt CSingerMgr : public CMediaSet
{
	friend class CModel;

public:
	CSingerMgr(CMediaSet& RootMediaSet, CDao& dao)
		: CMediaSet(L"歌手库", &RootMediaSet)
		, m_dao(dao)
	{
	}

private:
	CDao& m_dao;

	list<CSingerGroup> m_lstGroups;

	list<CSinger> m_lstRootSingers;

public:
	BOOL Init();

	void GetAllSinger(TD_MediaSetList& lstSinger);

	void GetSinger(const wstring& strDir, map<wstring, pair<UINT, wstring>>& mapSingerInfo);

	CSinger* GetSinger(const wstring& strMediaPath);

	void GetSubSets(TD_MediaSetList& lstSubSets) override;

	CMediaSet* GetLastObject()
	{
		return NULL;
	}

	CSingerGroup *AddGroup();

	BOOL RemoveGroup(UINT uSingerGroupID);

	CSinger *AddSinger(CMediaRes& SrcPath, CSingerGroup *pGroup, bool bInitAlbum);

	BOOL RemoveSinger(UINT uSingerID);

	BOOL UpdateSingerPos(UINT uSingerID, int nPos, int nDstGroupID);


	CAlbum *AddAlbum(CSinger& Singer, wstring strName=L"", const TD_IMediaList *plstAlbumItem = NULL);

	BOOL RemoveAlbum(UINT uID);

	CAlbum *RepositAlbum(CAlbum& Album, int nNewPos);

	BOOL AddAlbumItems(const list<wstring>& lstMediaPaths, CAlbum& Album, int nPos=-1);
	BOOL AddAlbumItems(const TD_IMediaList& lstMedias, CAlbum& Album, int nPos = -1);

	BOOL RemoveAlbumItems(const TD_AlbumItemList& lstAlbumItems);

	BOOL SetBackAlbumItems(const TD_MediaList& lstAlbumItem);

	CSinger* RepositSinger(CSinger& Singer, CMediaSet& Target, bool bDropPositionFlag);

	int RepositAlbumItem(CAlbum& Album, const TD_IMediaList& lstMedias, UINT uTargetPos);
};
