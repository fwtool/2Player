#pragma once

#include <DataMgr.h>

#include <Player.h>

#define __DemandCount 30

class __ModelExt CPlayMgr
{
	friend class CModel;

public:
	CPlayMgr(CPlaylistMgr& PlaylistMgr, CDataMgr& DataMgr, interface IModelObserver& ModelObserver);
	
	UINT m_uPlayingItem = 0;

private:
	CPlayer m_Player;

	CPlaylistMgr& m_PlaylistMgr;

	CPlaylist& m_Playlist;
	
	CDataMgr& m_DataMgr;

	IModelObserver& m_ModelObserver;
	
	set<UINT> m_setPlayedIDs;

private:
	void _playNext(int nFlag);

	void _refresh();

	bool _clear();

	bool _pause(const wstring& strPath, const function<void(wstring&)>& cb);

public:
	CMediaSet& getPlaylist()
	{
		return m_Playlist;
	}

	ArrList<CPlayItem>& getPlayingItems()
	{
		return m_Playlist.m_lstPlayItems;
	}

	CPlayer& getPlayer()
	{
		return m_Player;
	}

	E_PlayStatus GetPlayStatus();

	bool init(const CB_PlayFinish& cbFinish);

	bool clear();

	bool insert(const TD_IMediaList& lstMedias, bool bPlay, int iPos = -1);

	bool assign(const TD_IMediaList& lstMedias);

	int move(const TD_PlayItemList& lstPlayItems, UINT uPos);
	
	bool remove(const TD_PlayItemList& lstPlayItems);

	bool checkPlayedID(UINT uID);

	int getRandomPlayItem();

	bool play(int iItem=-1);
	
	void playNext();
	void playLast();
	
	void renameFile(const wstring& strPath, const function<wstring(const wstring&)>& cb);
	bool removeFile(const wstring& strPath, const function<bool()>& cb);
	void moveFile(const wstring& strPath, const wstring& strNewPath, const function<bool()>& cb);

	bool demandMedia(const TD_MediaList& lstMedias, UINT uDemandCount = __DemandCount);
	bool demandMedia(CMediaSet& rootMediaSet, E_MediaSetType nType);
	bool demandMediaSet(CMediaSet& rootMediaSet, E_MediaSetType nType);
};
