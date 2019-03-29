#pragma once

class __MediaLibExt CPlayItem : public CMedia
{
public:
	CPlayItem(int nID, const wstring& strPath, time_t time, class CPlaylist& Playlist);
	
private:
	void onAsyncTask() override;

	bool CheckRelatedMediaSetChanged(const tagMediaSetChanged& MediaSetChanged) override;

	wstring _GetDisplayName();

	wstring GetExportFileName() override
	{
		return _GetDisplayName() + GetExtName();
	}

	void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage) override;

public:
	class CPlaylist* GetPlaylist() const;

	wstring GetPlaylistName() const;

	inline bool isPlayingItem() const
	{
		return 0 == m_pParent->m_uID;
	}

	int getSingerImg() const;
};

class __MediaLibExt CPlaylist : public CMediaSet
{
	friend class CModel;
	friend class CPlaylistMgr;
	friend class CPlayMgr;
	friend class CPlayItem;
	
public:
	CPlaylist()
	{
	}

	CPlaylist(CMediaSet& parent, int nID, const wstring& strName);

	CPlaylist(const CPlaylist& other);

private:
	ArrList<CPlayItem> m_lstPlayItems;
	
public:
	UINT GetPlayItemCount()
	{
		return m_lstPlayItems.size();
	}

	void GetMedias(TD_MediaList& lstMedias) override
	{
		lstMedias.add(m_lstPlayItems);
	}

private:
	int GetPlayItemPos(CPlayItem& PlayItem);

	wstring _GetDisplayName();

	void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage) override
	{
		iImage = (int)E_GlobalImage::GI_Playlist;
		vecText.push_back(_GetDisplayName().c_str());
	}

	wstring GetExportPosition() override
	{
		return m_pParent->GetExportPosition() + fsutil::backSlant + this->_GetDisplayName();
	}
};
