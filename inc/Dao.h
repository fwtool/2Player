#pragma once

#include <util/util.h>

#ifdef __DalPrj
#define __DalExt __declspec(dllexport)
#else
#define __DalExt __declspec(dllimport)
#endif

#define __ID_GROUP_ROOT 0

enum E_OptionIndex
{
	OI_NULL = 0

	, OI_RootDir

	, OI_PlaySpiritLeft
	, OI_PlaySpiritTop
	, OI_PlaySpiritVolum

	, OI_PlaySpiritSkin

	, OI_IfRandomPlay

	, OI_AlarmHour
	, OI_AlarmMinute

	, OI_PlayingItem
	
	, OI_AttachDir

	, OI_HideMenuBar

	, OI_FullScreen
};

struct tagPlayItem
{
	int nID = 0;
	int nPlaylistID = 0;
	wstring strPath;
	int time = 0;
};

struct tagPlaylist
{
	int nID = 0;
	wstring strName;

	SArray<tagPlayItem> arrPlayItem;
};

struct tagAlbumItem
{
	int nID = 0;
	wstring strPath;
	int time = 0;
};

struct tagAlbum
{
	int nID = 0;
	wstring strName;

	SArray<tagAlbumItem> arrAlbumItem;
};

struct tagSinger
{
	int nID = 0;
	wstring strName;
	wstring strPath;
	int nPos = 0;

	SArray<tagAlbum> arrAlbum;
};

struct tagSingerGroup
{
	int nID = 0;
	wstring strName;

	SArray<tagSinger> arrSinger;
};

struct tagModifyedMedia
{
	wstring strOldPath;

	wstring strNew;

	wstring strMediaSetName;

	wstring strSingerName;
};

typedef struct tagNewPlayItem
{
	wstring strPath;

	wstring strPlaylistName;
} tagDeletedPlayItem;

typedef struct tagNewAlbumItem
{
	wstring strPath;

	wstring strAlbumName;

	wstring strSingerName;

	wstring strSingerDir;

	int iSingerID = 0;

} tagDeletedAlbumItem;

struct tagMovedMedia
{
	wstring strPath;

	wstring strOldMediaSetName;
	wstring strNewMediaSetName;

	wstring strSingerName;
};

struct tagCompareBackupResult
{
	tagCompareBackupResult()
	{
	}

	tagCompareBackupResult(const wstring t_strSrcFile, const wstring t_strDstFile=L"")
		: strSrcFile(t_strSrcFile)
		, strDstFile(t_strDstFile)
	{
	}

	wstring strSrcFile;
	wstring strDstFile;

	SArray<tagNewPlayItem> arrNewPlayItem;
	SArray<tagNewAlbumItem> arrNewAlbumItem;

	SArray<tagDeletedPlayItem> arrDeletedPlayItem;
	SArray<tagDeletedAlbumItem> arrDeletedAlbumItem;

	SArray<tagModifyedMedia> arrModifyedMedia;

	SArray<tagMovedMedia> arrMovedMedia;
};

class __DalExt CDao
{
public:
	class __DalExt CTransGuard : public CDBTransGuard
	{
	public:
		CTransGuard(CDao& dao)
			: CDBTransGuard(&dao.m_db)
		{
		}

		CTransGuard(CDao *pdao)
			: CDBTransGuard(NULL!=pdao?&pdao->m_db:NULL)
		{
		}
	};

public:
	CDao(IDB& db)
		: m_db(db)
	{
	}

	~CDao()
	{
		if (NULL != m_pRst)
		{
			delete m_pRst;
		}
	}

private:
	IDB& m_db;

	IDBResult *m_pRst = NULL;

public:
	bool execute(const string& strSql);
	bool execute(const wstring& strSql);

	IDBResult *query(const string& strSql);
	IDBResult *query(const wstring& strSql);

	bool queryPlaylist(SArray<tagPlaylist>& arrPlaylist);

	int addPlaylist(const wstring& strName);

	bool updatePlaylistPos(UINT uID, UINT uOldPos, UINT uNewPos);

	bool updatePlaylistName(UINT uID, const wstring& strName);

	bool deletePlaylist(UINT uID);

	int GetMaxPlayItemPos(UINT uPlaylistID);

	using CB_addPlayItem = function<void(UINT uPlayItemID, wstring strPath, time_t time)>;
	bool addPlayItem(const SArray<wstring>& lstOppPaths, UINT uPlaylistID, int nPos, const CB_addPlayItem& cb);

	bool deletePlayItem(const list<UINT>& lstIDs);
	bool deletePlayItem(UINT uPlaylistID);

	bool updatePlayItemPath(UINT uPlayItemID, const wstring& strPath);

	bool setbackPlayItem(UINT uPlaylistID, const list<UINT>& lstIDs);

public:
	bool querySinger(SArray<tagSingerGroup>& arrSingerGroup);

	int addSingerGroup(const wstring& strName);

	bool updateSingerGroupName(UINT uSingerGroupID, const wstring& strName);

	bool deleteSingerGroup(UINT uSingerGroupID);

	int addSinger(UINT uGroupID, const wstring& strSingerName, const wstring& strPath, int nPos);

	bool updateSingerName(UINT uSingerID, const wstring& strName);

	bool updateSingerPath(const map<UINT, wstring>& mapSingerDirChanged);
	
	bool deleteSinger(UINT uSingerID);

	bool updateSingerPos(UINT uSingerID, int nPos, int nDstGroupID);

	int addAlbum(const wstring& strName, UINT uSingerID, UINT pos);

	bool updateAlbumName(UINT uAlbumID, const wstring& strName);

	bool deleteAlbum(UINT uAlbumID, UINT uSingerID);

	bool updateAlbumPos(UINT uAlbumID, int nOldPos, int nNewPos, UINT uSingerID);

	int queryMaxAlbumItemPos(int nAlbumID);

    using CB_addAlbumItem = function<void(UINT uPlayItemID, const wstring& strPath, time_t time)>;
	bool addAlbumItem(const list<wstring>& lstOppPaths, UINT uAlbumID, int nPos, const CB_addAlbumItem& cb);

	bool updateAlbumItemPath(UINT uAlbumItemID, const wstring& strPath);

	bool deleteAlbumItem(UINT uAlbumItem);
	bool deleteAlbumItem(const list<UINT>& lstAlbumItemIDs);

	bool setbackAlbumItem(UINT uAlbumID, const list<UINT>& lstAlbumItemID);

public:
	wstring getNewPlaylistName(const wstring& strBaseName);
	wstring getNewSingerGroupName(const wstring& strBaseName);
	wstring getNewAlbumName(UINT uSingerID, const wstring& strBaseName);

	bool queryAlarmmedia(vector<wstring>& vecAlarmmedia);
	bool addAlarmmedia(const SArray<wstring>& vecAlarmmedia);
	bool deleteAlarmmedia(const wstring& strPath);
	bool clearAlarmmedia();

	bool queryPlayRecordMaxTime(time_t& time);
	bool queryPlayRecord(time_t time, vector<pair<wstring, int>>& vecPlayRecord);
	bool clearPlayRecord();

	bool updateFiles(const wstring& strOldOppPath, const wstring& strNewOppPath, bool bDir);
	bool deleteFiles(const set<wstring>& setDeleteFiles);
	
	bool queryDifferent(tagCompareBackupResult& result);

	bool clearAll();

public:
	bool GetOption(E_OptionIndex eOptionIndex, bool& bOptionValue)
	{
		int iOptionValue = 0;
		if (!GetOption(eOptionIndex, iOptionValue))
		{
			return FALSE;
		}

		bOptionValue = 1 == iOptionValue;

		return TRUE;
	}

	template <typename T>
	bool GetOption(E_OptionIndex eOptionIndex, T& OptionValue)
	{
		wstring strSql = L"SELECT value FROM tbl_option WHERE id = " + to_wstring(eOptionIndex);
		IDBResult *pRst = query(strSql);
		__EnsureReturn(pRst, FALSE);

		if (1 == pRst->GetRowCount())
		{
			__EnsureReturn(pRst->GetData(0, 0, OptionValue), FALSE);
		}

		return TRUE;
	}

	template <typename T>
	bool SetOption(E_OptionIndex eOptionIndex, const T& OptionValue)
	{
		wstringstream ssSql;
		ssSql << L"INSERT OR REPLACE INTO tbl_option \
				   VALUES(" << eOptionIndex << L", \"" << OptionValue << L"\")";

		return m_db.Execute(ssSql.str());
	}

private:
	wstring getNextName(const wstring& strTableName, const wstring& strNameColumn, const wstring& strBaseName);

	bool _getMaxValue(const wstring& strTableName, const wstring& strNameColumn, int& iRet);
};

using CDaoTransGuard = CDao::CTransGuard;
