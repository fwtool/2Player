
#pragma once

class __MediaLibExt CMediaSet : public tagMediaInfo, public CListObject, public CTreeObject
{
public:
	CMediaSet(const wstring& strName = L"", CMediaSet *pParent = NULL, int nID = 0, E_MediaSetType eType = E_MediaSetType::MST_Null)
		: tagMediaInfo(pParent, strName, nID)
		, m_eType(eType)
	{
		if (pParent)
		{
			pParent->_AddSubSet(*this);
		}
	}

	~CMediaSet()
	{
	}

public:
	E_MediaSetType m_eType;

private:
	TD_MediaSetList m_lstSubSets;
	TD_MediaList m_lstMedias;

private:
	void _AddSubSet(CMediaSet& MediaSet)
	{
		m_lstSubSets.add(MediaSet);
	}

	bool GetRenameText(wstring& stRenameText) const override
	{
		stRenameText = m_strName;
		return true;
	}

	bool _FindMedia(CMedia& Media, const tagFindMediaPara& FindPara, struct tagFindMediaResult& FindResult);

public:
	UINT GetIndex() const;

	virtual wstring GetBaseDir() const;

	void AddMedia(CMedia& Media)
	{
		m_lstMedias.add(&Media);
	}

	BOOL RemoveMedia(CMedia& Media);

	virtual void GetSubSets(TD_MediaSetList& lstSubSets)
	{
		lstSubSets.add(m_lstSubSets);
	}

	virtual void GetMedias(TD_MediaList& lstMedias)
	{
		lstMedias.add(m_lstMedias);
	}

	void GetAllMediaSets(E_MediaSetType eType, TD_MediaSetList& lstMediaSets);

	void GetAllMedias(TD_MediaList& lstMedias);

	CMediaSet* GetMediaSet(UINT uMediaSetID, E_MediaSetType eMediaSetType);

	CMedia* GetMedia(UINT uMediaID, E_MediaSetType eMediaSetType);

	void GetTreeChilds(TD_TreeObjectList& lstChilds) override;

	void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage) override
	{
		vecText.push_back(m_strName.c_str());
	}

	wstring GetTreeText() const override
	{
		return m_strName;
	}

	wstring GetLogicPath(const wstring& strDelimiter = __CNDotDelimiter);

	virtual wstring GetExportPosition()
	{
		return GetLogicPath(wstring({ fsutil::backSlant }));
	}
	
	bool SetAsyncTaskFlag();

	virtual bool FindMedia(const tagFindMediaPara& FindPara, struct tagFindMediaResult& FindResult);
};

struct tagFindMediaResult
{
	UINT uResult = 0;

	TD_MediaList lstRetMedias;

	CMediaSet *pRetSinger = NULL;

	map<UINT, wstring> mapSingerDirChanged;
};
