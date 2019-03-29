#pragma once

class __MediaLibExt CAlbumItem : public CMedia
{
public:
	CAlbumItem(int nID, const wstring& strPath, time_t time, class CAlbum& Album);

private:
	void onAsyncTask() override;

	void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage) override;

public:
	class CAlbum* GetAlbum() const;
	
	wstring GetAlbumName() const;

	class CSinger* GetSinger() const;

	wstring GetSingerName() const;
};

class __MediaLibExt CAlbum : public CMediaSet
{
	friend class CSingerMgr;

public:
	CAlbum(int nID=0, const wstring& strName=L"", class CSinger *pSinger=NULL)
		: CMediaSet(strName, (CMediaSet*)pSinger, nID, E_MediaSetType::MST_Album)
	{
	}

private:
	list<CAlbumItem> m_lstAlbumItems;

public:
	UINT GetAlbumItemCount()
	{
		return m_lstAlbumItems.size();
	}

	void GetMedias(TD_MediaList& lstMedias) override
	{
		lstMedias.add(m_lstAlbumItems);
	}

	class CSinger& GetSinger() const;

private:
	void AddAlbumItems(const list<CAlbumItem>& lstAlbumItems);

	void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage) override;

	wstring GetExportPosition() override;
};


class __MediaLibExt CSinger : public CMediaSet
{
	friend class CSingerMgr;

public:
	CSinger(CMediaSet& parent, int nID = 0, const wstring& strName = L"", const wstring& strDir = L"", int nPos = 0);

public:
	UINT m_uImgPos = 0;

private:
	wstring m_strDir;

	list<CAlbum> m_lstAlbums;

	int m_nPos = 0;

public:
	void GetSubSets(TD_MediaSetList& lstSubSets) override
	{
		lstSubSets.add(m_lstAlbums);
	}

	wstring GetBaseDir() const override
	{
		return m_strDir;
	}

private:
	int GetTreeImage() override;

	bool FindMedia(const tagFindMediaPara& FindPara, tagFindMediaResult& FindResult) override;
};

class __MediaLibExt CSingerGroup : public CMediaSet
{
	friend class CSingerMgr;

public:
	CSingerGroup(CMediaSet& parent, int nID=0, const wstring& strName = L"");

private:
	list<CSinger> m_lstSingers;

private:
	void GetSubSets(TD_MediaSetList& lstSubSets) override
	{
		lstSubSets.add(m_lstSingers);
	}

	int GetTreeImage() override
	{
		return (int)E_GlobalImage::GI_SingerGroup;
	}
};
