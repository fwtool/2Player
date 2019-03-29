
#pragma once

#include "CueFile.h"

class __MediaLibExt CMediaRes : public IMedia, public CPathObject
{
public:
	CMediaRes()
	{
	}

	CMediaRes(const tagFindData& findData, CPath *pParentDir, E_MediaFileType eFileType = E_MediaFileType::MFT_Null);

	CMediaRes(const wstring& strDir);

private:
	struct tagMediaTag
	{
		wstring strTitle;
		wstring strArtist;
		wstring strAlbum;
	} m_MediaTag;
	
	ArrList<CCueFile> m_alSubCueFile;

	SHashMap<wstring, UINT> m_mapSubCueFile;

	LPCCueFile m_pCueFile = NULL;

private:
	bool _loadCue(const wstring& strFileName);

	CPath *NewSubPath(const tagFindData& findData, CPath *pParentDir) override;

	bool ReadTag();
	void ReadMP3Tag(FILE *lpFile);
    bool ReadFlacTag();

	void onAsyncTask() override;

	bool CheckRelatedMediaSetChanged(const tagMediaSetChanged& MediaSetChanged) override;

	virtual bool GetRenameText(wstring& stRenameText) const override;

	void OnListItemRename(const wstring& strNewName) override;
	
	int _getImage();

public:
	CMediaRes* GetParent() const;

	const ArrList<CCueFile>& getSubCueFile()
	{
		return m_alSubCueFile;
	}

	CRCueFile getSubCueFile(CMediaRes& MediaRes);

	CRCueFile getCueFile();
	
	virtual wstring GetPath() const override;

	wstring GetAbsPath() const override;

	bool IsDir() const override
	{
		return m_bDir;
	}

	virtual wstring GetName() const override
	{
		return m_strName;
	}

	int GetFileSize() const override
	{
		return (int)m_uFileSize;
	}
	
	void SetDirRelatedSinger(UINT uSingerID, const wstring& strSingerName, bool& bChanged);

	void GenListItem(vector<wstring>& vecText, int& iImage, bool bGenRelatedSinger);

	virtual CMediaRes *FindSubPath(wstring strSubPath, bool bDir)
	{
        return (CMediaRes*)CPathObject::FindSubPath(strSubPath, bDir);
	}

	void Clear() override;
};
