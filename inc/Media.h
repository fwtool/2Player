
#pragma once

#define __time32_t time_t

struct __MediaLibExt tagMediaInfo
{
	tagMediaInfo(CMediaSet *pParent = NULL, const wstring& strName = L"", int nID = 0)
		: m_pParent(pParent)
		, m_strName(strName)
		, m_uID(nID)
	{
	}

	CMediaSet *m_pParent;

	wstring m_strName;

	UINT m_uID = 0;
};

class __MediaLibExt CMedia : public IMedia, public tagMediaInfo, public CListObject
{
	friend class __model;

public:
	CMedia(CMediaSet *pParent = NULL, int nID = 0, const wstring& strPath = L"", time_t time = 0)
		: IMedia(strPath)
		, tagMediaInfo(pParent, L"", nID)
		, m_addTime(time)
	{
		this->_UpdatePath(strPath);
	}

	virtual ~CMedia()
	{
	}

protected:
	wstring m_strParentDir;
	
	wstring m_strTitle;

	CMediaTime m_addTime;

	int m_iFileSize = 0;

	int m_iDuration = 0;

protected:
	virtual void onAsyncTask() override;

public:
	wstring GetBaseDir() const;
	
	wstring GetDir() const
	{
		return GetBaseDir() + m_strParentDir;
	}
	
	wstring GetPath() const override
	{
		return GetDir() + fsutil::backSlant + m_strName;
	}

	wstring GetAbsPath() const override
	{
		__EnsureReturn(g_pRootMediaRes, L"");
		return g_pRootMediaRes->toAbsPath(GetPath());
	}

	wstring GetName() const override
	{
		return m_strName;
	}

	const wstring& GetTitle() const
	{
		return m_strTitle;
	}

	const CMediaTime& GetAddTime() const
	{
		return m_addTime;
	}

	int CheckFileSize()
	{
		return m_iFileSize = fsutil::GetFileSize(this->GetAbsPath());
	}

	int GetFileSize() const override
	{
		return m_iFileSize;
	}

	wstring GetDurationString() const;

	virtual int GetDuration() const
	{
		return m_iDuration;
	}

	virtual void SetDuration(int iDuration)
	{
		m_iDuration = iDuration;
	}

	//void UpdateFileInfo(int iFileSize, int iDuration);

	CMediaSet *GetMediaSet() override
	{
		return m_pParent;
	}

	virtual wstring GetExportFileName() override
	{
		return m_strTitle;
	}
	
	void UpdatePath(const wstring& strPath);

	CMediaRes *getMediaRes() const;

	CRCueFile getCueFile() const;

private:
	void _UpdatePath(const wstring& strPath);

	bool GetRenameText(wstring& stRenameText) const override
	{
		stRenameText = m_strTitle;
		return true;
	}

	void OnListItemRename(const wstring& strNewName) override;
};
