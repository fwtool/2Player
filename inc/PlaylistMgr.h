#pragma once

#define ID_PLAYLIST_NULL 0

class __ModelExt CPlaylistMgr : public CMediaSet
{
public:
	CPlaylistMgr(CMediaSet& RootMediaSet, CDao& dao)
		: CMediaSet(L"播放列表", &RootMediaSet)
		, m_dao(dao)
		, m_Playinglist(*this, ID_PLAYLIST_NULL, L"正在播放")
	{
	}

private:
	CDao& m_dao;

	list<CPlaylist> m_lstPlaylists;

	CPlaylist m_Playinglist;

public:
	CPlaylist& GetPlayinglist()
	{
		return m_Playinglist;
	}
	
	list<CPlaylist>& GetPlaylists()
	{
		return m_lstPlaylists;
	}

	BOOL Init();

	void GetSubSets(TD_MediaSetList& lstSubSets) override;

	CPlaylist *AddPlaylist(UINT uPos);

	BOOL RemovePlaylist(UINT uID);

	CPlaylist *RepositPlaylist(UINT uID, UINT uNewPos);

	BOOL AddPlayItems(const TD_IMediaList& lstMedias, CPlaylist& Playlist, int nPos = -1);
	BOOL AddPlayItems(const SArray<wstring>& lstOppPaths, CPlaylist& Playlist, int nPos = -1);
	BOOL RemovePlayItems(const TD_PlayItemList& arrPlayItems);
	BOOL RemoveAllPlayItems(CPlaylist& Playlist);

	int RepositPlayItem(CPlaylist& Playlist, const TD_IMediaList& lstMedias, UINT uTargetPos);

	BOOL SetBackPlayItems(CPlaylist& Playlist, const TD_MediaList& lstPlayItems);
};
