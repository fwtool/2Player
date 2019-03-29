#pragma once

struct tagFileTitle
{
	tagFileTitle() {}

	tagFileTitle(const wstring& t_strFileTitle)
	{
		strFileTitle = t_strFileTitle;

		for (const wstring& str : {L"[mqms2]", L"[mqms]"})
		{
			wstring::size_type pos = strFileTitle.find(str);
			if (wstring::npos != pos)
			{
				strFileTitle.erase(pos, str.size());
				break;
			}
		}

		for (auto& chr : strFileTitle)
		{
			if (L'_' == chr)
			{
				chr = L' ';
			}
		}

		strFileTitle = util::StrLowerCase(util::trim(strFileTitle));

		vector<wstring> vecFileTitle;
		util::SplitString(strFileTitle, L'-', vecFileTitle, true);
		setFileTitle.add(vecFileTitle);
	}

	wstring strFileTitle;
	SSet<wstring> setFileTitle;
};

struct tagMediaResInfo
{
	tagMediaResInfo(const wstring& t_strPath, const wstring& t_strFileSize)
		: strPath(t_strPath)
		, FileTitle(fsutil::getFileTitle(strPath))
		, strFileSize(t_strFileSize)
	{
	}

	tagMediaResInfo(const wstring& t_strPath)
		: strPath(t_strPath)
		, FileTitle(fsutil::getFileTitle(strPath))
		, strFileSize(IMedia::GetFileSizeString(fsutil::GetFileSize(strPath)))
	{
	}

    bool operator==(const tagMediaResInfo& other) const
    {
        return &other == this;
    }

	wstring strPath;

	tagFileTitle FileTitle;

	wstring strFileSize;
};

class __ModelExt CSearchMediaInfo
{
private:
	tagFileTitle m_FileTitle;

	wstring m_strFileSize;

	wstring m_strSingerName;

public:
	wstring m_strAbsPath;

	TD_MediaList m_lstMedias;

public:
	CSearchMediaInfo() {}

	CSearchMediaInfo(CMedia& Media, CSinger& singer);

	CSearchMediaInfo(CMedia& Media, const SSet<wstring>& setSingerName);

	wstring GetFileSize();

	bool matchMediaRes(tagMediaResInfo& MediaResInfo);
};

typedef map<wstring, CSearchMediaInfo> TD_SearchMediaInfoMap;

class __ModelExt CSearchMediaInfoGuard
{
public:
	CSearchMediaInfoGuard(CSingerMgr& SingerMgr);

public:
	void genSearchMediaInfo(CMedia& Media, TD_SearchMediaInfoMap& mapSearchMediaInfo);

private:
	CSingerMgr& m_SingerMgr;

	SSet<wstring> m_setSingerName;
};

enum class E_MatchResult
{
	MR_Yes
	, MR_No
	, MR_Ignore
};

using CB_AutoMatchConfirm = function<E_MatchResult(CSearchMediaInfo&, tagMediaResInfo&)>;

using CB_AutoMatchProgress = function<bool(const wstring& strDir)>;

class __ModelExt CAutoMatch
{
public:
	CAutoMatch(CMediaRes& RootMediaRes, const CB_AutoMatchProgress& cbProgress, const CB_AutoMatchConfirm& cbConfirm)
		: m_RootMediaRes(RootMediaRes)
		, m_cbProgress(cbProgress)
		, m_cbConfirm(cbConfirm)
	{
	}

private:
	CMediaRes& m_RootMediaRes;

	CB_AutoMatchProgress m_cbProgress;
	CB_AutoMatchConfirm m_cbConfirm;

	map<wstring, TD_SearchMediaInfoMap> m_mapSearchMedia;
	
public:
	void autoMatchMedia(CSingerMgr& SingerMgr, CMediaRes& SrcPath, const TD_MediaList& lstMedias);

private:
	void FilterBasePath(CMediaRes& SrcPath, CMediaRes *pDir, list<wstring>& lstSubPaths
		, list<wstring>& lstPaths, list<wstring>& lstNewSubPaths);

	void enumMediaRes(CMediaRes& SrcPath, CMediaRes *pDir, list<wstring>& lstPaths, list<wstring>& lstSubPaths);

	void matchMedia(CMediaRes& MediaRes, tagMediaResInfo& MediaResInfo, TD_SearchMediaInfoMap& mapSearchMedias);
};
