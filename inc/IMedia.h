
#pragma once

enum class E_MediaFileType
{
	MFT_Null = 0
	
	, MFT_MP3
	, MFT_FLAC
	, MFT_WAV
	, MFT_APE
	, MFT_WMA
};

class __MediaLibExt IMedia
{
public:
	IMedia() {}

	IMedia(const wstring& strPath);

	IMedia(E_MediaFileType eFileType);

	virtual ~IMedia() {}

protected:
	E_MediaFileType m_eFileType = E_MediaFileType::MFT_Null;

private:
	bool m_bAsyncTaskFlag = false;

	tagRelatedMediaSet m_lpRelatedMediaSet[4];

	tagRelatedMediaSet& _getRelatedMediaSet(E_MediaSetType eMediaSetType)
	{
		return m_lpRelatedMediaSet[(UINT)eMediaSetType];
	}

	const tagRelatedMediaSet& _getRelatedMediaSet(E_MediaSetType eMediaSetType) const
	{
		return m_lpRelatedMediaSet[(UINT)eMediaSetType];
	}

public:
	virtual wstring GetPath() const = 0;
	virtual wstring GetAbsPath() const = 0;

	virtual bool IsDir() const
	{
		return false;
	}

	virtual wstring GetName() const = 0;

	E_MediaFileType GetFileType() const
	{
		return m_eFileType;
	}

	wstring GetFileTypeString() const;

	wstring GetExtName() const;

	virtual int GetFileSize() const = 0;
	wstring GetFileSizeString(bool bIgnoreByte = true) const;
	static wstring GetFileSizeString(int iFileSize, bool bIgnoreByte=true);

	int CheckDuration();

	virtual CMediaSet *GetMediaSet()
	{
		return NULL;
	}

	E_MediaSetType GetMediaSetType();

	virtual wstring GetExportFileName()
	{
		return GetName();
	}

	void Explore()
	{
		list<wstring> lstPath{ GetAbsPath() };
		fsutil_win::ExplorePath(lstPath);
	}

	void SetAsyncTaskFlag()
	{
		m_bAsyncTaskFlag = true;
	}

	bool DoAsyncTask();

	bool UpdateRelatedMediaSet(const tagMediaSetChanged& MediaSetChanged);

	int GetRelatedMediaID(E_MediaSetType eMediaSetType) const
	{
		return _getRelatedMediaSet(eMediaSetType).iMediaID;
	}

	int GetRelatedMediaSetID(E_MediaSetType eMediaSetType) const
	{
		return _getRelatedMediaSet(eMediaSetType).iMediaSetID;
	}

	const wstring& GetRelatedMediaSetName(E_MediaSetType eMediaSetType)
	{
		return _getRelatedMediaSet(eMediaSetType).strMediaSetName;
	}

	void SetRelatedMediaSet(E_MediaSetType eMediaSetType, int iMediaSetID, const wstring& strMediaSetName, int iMediaID = -1);

	void SetRelatedMediaSet(CMediaSet& MediaSet);

	void SetRelatedMediaSet(CMedia& Media);

	void ClearRelatedMediaSet(E_MediaSetType eMediaSetType);

protected:
	void updateFileType(const wstring& strPath);

	virtual bool CheckRelatedMediaSetChanged(const tagMediaSetChanged& MediaSetChanged) { return false; }

	virtual void onAsyncTask()=0;
};
