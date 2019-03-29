#pragma once

interface IMediaObserver
{
	virtual void renameMedia(IMedia& media, const wstring& strNewName) = 0;

	virtual class CMedia *findRelatedMedia(IMedia& media, E_MediaSetType eMediaSetType) = 0;

	virtual int checkMedia(const wstring& strAbsPath) = 0;

	virtual UINT getSingerImgPos(UINT uSingerID) = 0;
};

class CAttachDir : public CMediaRes
{
public:
	CAttachDir(const wstring& strDir)
		: CMediaRes(strDir)
	{
	}

public:
	wstring GetName() const override
	{
		return fsutil::GetFileName(m_strName);
	}

	wstring GetPath() const override
	{
		return fsutil::backSlant + GetName();
	}

	bool GetRenameText(wstring& stRenameText) const
	{
		return false;
	}
};

class __MediaLibExt CRootMediaRes : public CMediaRes
{
public:
	static E_MediaFileType GetMediaFileType(const wstring& strFile);

	static wstring GetMediaFileType(E_MediaFileType eMediaFileType);

public:
	CRootMediaRes(IMediaObserver& MediaObserver);

	void init(const wstring& strDir, const vector<wstring>& vecAttachDir);

	CMediaRes* FindSubPath(wstring strSubPath, bool bDir) override;

private:
	vector<wstring> m_vecAttachDir;

	map<wstring, CAttachDir*> m_mapAttachDir;
	
	TD_PathList& _findFile() override;
	
public:
	IMediaObserver& m_MediaObserver;

	wstring toAbsPath(const wstring& strSubPath) const;

	wstring toOppPath(const wstring& strAbsPath) const;

	bool checkIndependentDir(const wstring& strAbsDir, bool bCheckAttachDir);
};

extern CRootMediaRes *g_pRootMediaRes;
