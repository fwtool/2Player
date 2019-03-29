
#include "Dao.h"

bool CDao::execute(const string& strSql)
{
	return m_db.Execute(strSql);
}

bool CDao::execute(const wstring& strSql)
{
	return m_db.Execute(strSql);
}

IDBResult *CDao::query(const string& strSql)
{
	if (NULL != m_pRst)
	{
		delete m_pRst;
	}

	m_pRst = m_db.Query(strSql);

	return m_pRst;
}

IDBResult *CDao::query(const wstring& strSql)
{
	if (NULL != m_pRst)
	{
		delete m_pRst;
	}

	m_pRst = m_db.Query(strSql);

	return m_pRst;
}

bool CDao::queryPlaylist(SArray<tagPlaylist>& arrPlaylist)
{
	arrPlaylist.add(tagPlaylist());

	wstring strSql = L"SELECT id, name FROM tbl_playlist ORDER BY pos";
	IDBResult *pRst = query(strSql);
	__EnsureReturn(pRst, false);
	
	map<int, size_t> mapPlaylistPos;
	int nID = 0;
	wstring strName;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		__EnsureReturn(pRst->GetData(uRow, 0, nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, strName), false);
		
		tagPlaylist Playlist;
		Playlist.nID = nID;
		Playlist.strName = strName;
		arrPlaylist.add(Playlist);
		
		mapPlaylistPos[nID] = uRow+1;
	}

	strSql = L"SELECT id, playlist_id, path, create_time FROM tbl_playitem ORDER BY pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);

	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		tagPlayItem PlayItem;
		__EnsureReturn(pRst->GetData(uRow, 0, PlayItem.nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, PlayItem.nPlaylistID), false);
		__EnsureReturn(pRst->GetData(uRow, 2, PlayItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 3, PlayItem.time), false);

		arrPlaylist.get(mapPlaylistPos[PlayItem.nPlaylistID], [&](tagPlaylist& Playlist) {
			Playlist.arrPlayItem.add(PlayItem);
		});
	}

	return true;
}

int CDao::addPlaylist(const wstring& strName)
{
	CDBTransGuard transGuard(m_db);

	wstring strSql = L"UPDATE tbl_playlist SET pos = pos+1";
	__EnsureReturn(execute(strSql), -1);

	wstringstream ssSql;
	ssSql << L"INSERT INTO tbl_playlist(name, pos) \
			   VALUES(\"" << strName << L"\", 0)";
	__EnsureReturn(execute(ssSql.str()), -1);

	int iMaxID = 0;
	__EnsureReturn(_getMaxValue(L"tbl_playlist", L"id", iMaxID), -1);

	return iMaxID;
}

bool CDao::updatePlaylistPos(UINT uID, UINT uOldPos, UINT uNewPos)
{
	int nFlag = uOldPos < uNewPos ? -1 : 1;

	wstringstream ssSql;
	ssSql << L"UPDATE tbl_playlist SET pos = pos + " << nFlag << L" \
			   WHERE pos BETWEEN " << min(uOldPos, uNewPos) << L" AND " << max(uOldPos, uNewPos) << L";";
	ssSql << L"UPDATE tbl_playlist SET pos = " << uNewPos << L" WHERE id = " << uID;
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::updatePlaylistName(UINT uID, const wstring& strName)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_playlist SET name = \"" << strName << L"\" \
			WHERE id = " << uID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::deletePlaylist(UINT uID)
{
	wstringstream ssSql;

	ssSql << L"UPDATE tbl_playlist SET pos = pos-1 \
			   WHERE pos > (SELECT pos FROM tbl_playlist WHERE id = " << uID << ")";

	ssSql << L"; DELETE FROM tbl_playitem \
			   WHERE playlist_id = " << uID;

	ssSql << L"; DELETE FROM tbl_playlist \
			   WHERE id = " << uID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

int CDao::GetMaxPlayItemPos(UINT uPlaylistID)
{
	wstringstream ssSql;
	ssSql << L"SELECT MAX(pos) FROM tbl_playitem WHERE playlist_id = " << uPlaylistID;
	IDBResult *pRst = query(ssSql.str());
	__EnsureReturn(pRst, false);

	int nMaxPos = 0;
	(void)pRst->GetData(0, 0, nMaxPos);

	return nMaxPos;
}

bool CDao::addPlayItem(const SArray<wstring>& lstOppPaths, UINT uPlaylistID, int nPos, const CB_addPlayItem& cb)
{
	CDBTransGuard transGuard(m_db);

	int nMaxPos = GetMaxPlayItemPos(uPlaylistID);

	int iMaxID = 0;
	_getMaxValue(L"tbl_playitem", L"id", iMaxID);

	auto addTime = time(0);

	wstringstream ssSql;
	for (auto& strOppPaths : lstOppPaths)
	{
		ssSql.str(L"");
		ssSql << L"INSERT INTO tbl_playitem(playlist_id, path, create_time, pos) \
				VALUES(" << uPlaylistID << L", \"" << strOppPaths << "\", " << addTime << L"," << ++nMaxPos << L")";

		if (0 == uPlaylistID)
		{
			ssSql << L"; INSERT INTO tbl_history(path, last_time) VALUES(\"" << strOppPaths << L"\", " << addTime << L")";
		}

		__EnsureReturn(execute(ssSql.str()), false);

		cb(++iMaxID, strOppPaths, addTime);
	}

	return true;
}

bool CDao::deletePlayItem(const list<UINT>& lstIDs)
{
	wstring strIDList = util::ContainerToStr(lstIDs, L",");

	wstring strSql = L"DELETE FROM tbl_playitem \
						 WHERE id IN(" + strIDList + L")";
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::deletePlayItem(UINT uPlaylistID)
{
	wstringstream ssSql;
	ssSql << L"DELETE FROM tbl_playitem \
			   WHERE playlist_id  = " << uPlaylistID;
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::updatePlayItemPath(UINT uPlayItemID, const wstring& strPath)
{
	wstringstream ssSql;

	ssSql << L"UPDATE tbl_playitem SET path = \"" + strPath + L"\" \
				WHERE id = " << uPlayItemID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::setbackPlayItem(UINT uPlaylistID, const list<UINT>& lstIDs)
{
	CDBTransGuard transGuard(m_db);

	int nMaxPos = GetMaxPlayItemPos(uPlaylistID);

	wstringstream ssSql;

	for (auto uID : lstIDs)
	{
		ssSql.str(L"");
		ssSql << L"UPDATE tbl_playitem SET pos = " << ++nMaxPos << L" WHERE id = " << uID;
		__EnsureReturn(execute(ssSql.str()), false);
	}

	return true;
}

bool CDao::querySinger(SArray<tagSingerGroup>& arrSingerGroup)
{
	arrSingerGroup.add(tagSingerGroup());

	wstring strSql = L"SELECT id, name FROM tbl_singergroup ORDER BY LOWER(name)";
	IDBResult *pRst = query(strSql);
	__EnsureReturn(pRst, false);

	wstring strName;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		tagSingerGroup SingerGroup;
		__EnsureReturn(pRst->GetData(uRow, 0, SingerGroup.nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, SingerGroup.strName), false);

		arrSingerGroup.add(SingerGroup);
	}

	SMap<int, tagSingerGroup*> mapSingerGroup;
	for (auto& SingerGroup : arrSingerGroup)
	{
		mapSingerGroup.set(SingerGroup.nID, &SingerGroup);
	}

	strSql = L"SELECT id, group_id, name, path, pos FROM tbl_singer ORDER BY pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);

	int nGroupID = 0;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		tagSinger Singer;
		__EnsureReturn(pRst->GetData(uRow, 0, Singer.nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, nGroupID), false);
		__EnsureReturn(pRst->GetData(uRow, 2, Singer.strName), false);
		__EnsureReturn(pRst->GetData(uRow, 3, Singer.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 4, Singer.nPos), false);

		mapSingerGroup.get(nGroupID, [&](tagSingerGroup *pSingerGroup) {
			pSingerGroup->arrSinger.add(Singer);
		});
	}
	
	SMap<int, tagSinger*> mapSinger;
	for (auto& SingerGroup : arrSingerGroup)
	{
		for (auto& Singer : SingerGroup.arrSinger)
		{
			mapSinger.set(Singer.nID, &Singer);
		}
	}

	strSql = L"SELECT id, singer_id, name FROM tbl_album ORDER BY pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);

	int nSingerID = 0;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		tagAlbum Album;
		__EnsureReturn(pRst->GetData(uRow, 0, Album.nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, nSingerID), false);
		__EnsureReturn(pRst->GetData(uRow, 2, Album.strName), false);

		mapSinger.get(nSingerID, [&](tagSinger *pSinger) {
			pSinger->arrAlbum.add(Album);
		});
	}

	SMap<int, tagAlbum*> mapAlbum;
	for (auto& SingerGroup : arrSingerGroup)
	{
		for (auto& Singer : SingerGroup.arrSinger)
		{
			for (auto& Album : Singer.arrAlbum)
			mapAlbum.set(Album.nID, &Album);
		}
	}

	strSql = L"SELECT id, album_id, path, create_time FROM tbl_albumitem ORDER BY pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);

	int nAlbumID = 0;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		tagAlbumItem AlbumItem;
		__EnsureReturn(pRst->GetData(uRow, 0, AlbumItem.nID), false);
		__EnsureReturn(pRst->GetData(uRow, 1, nAlbumID), false);
		__EnsureReturn(pRst->GetData(uRow, 2, AlbumItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 3, AlbumItem.time), false);

		mapAlbum.get(nAlbumID, [&](tagAlbum *pAlbum) {
			pAlbum->arrAlbumItem.add(AlbumItem);
		});
	}
	
	return true;
}

int CDao::addSingerGroup(const wstring& strName)
{
	wstringstream ssSql;
	ssSql << L"INSERT INTO tbl_singergroup(name) VALUES(\"" << strName << L"\")";
	__EnsureReturn(execute(ssSql.str()), -1);

	int iMaxID = 0;
	__EnsureReturn(_getMaxValue(L"tbl_singergroup", L"id", iMaxID), -1);
	
	return iMaxID;
}

bool CDao::updateSingerGroupName(UINT uSingerGroupID, const wstring& strName)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_singergroup SET name = \"" << strName << L"\" \
				WHERE id = " << uSingerGroupID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::deleteSingerGroup(UINT uSingerGroupID)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_singer SET group_id = " << __ID_GROUP_ROOT << " \
					, pos = pos + (SELECT COUNT(*) FROM tbl_singer WHERE group_id = " << __ID_GROUP_ROOT << ") \
				WHERE group_id = " << uSingerGroupID;
	__EnsureReturn(execute(ssSql.str()), false);

	ssSql.str(L"");
	ssSql << L"DELETE FROM tbl_singergroup \
			   WHERE id = " << uSingerGroupID;
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

int CDao::addSinger(UINT uGroupID, const wstring& strSingerName, const wstring& strPath, int nPos)
{
	wstringstream ssSql;
	ssSql << L"INSERT INTO tbl_singer(group_id, name, path, pos) \
			   VALUES(" << uGroupID << ", \"" << strSingerName << L"\", \"" << strPath << L"\", " << nPos << L")";
	__EnsureReturn(execute(ssSql.str()), -1);

	int iMaxID = 0;
	__EnsureReturn(_getMaxValue(L"tbl_singer", L"id", iMaxID), -1);

	return iMaxID;
}

bool CDao::updateSingerName(UINT uSingerID, const wstring& strName)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_singer SET name = \"" << strName << L"\" \
				WHERE id = " << uSingerID;

	return execute(ssSql.str());
}

bool CDao::updateSingerPath(const map<UINT, wstring>& mapSingerDirChanged)
{
	CDBTransGuard transGuard(m_db);

	for (auto& pr : mapSingerDirChanged)
	{
		wstringstream ssSql;
		ssSql << L"UPDATE tbl_singer SET path = \"" << pr.second << L"\" \
					WHERE id = " << pr.first;
		__EnsureReturn(execute(ssSql.str()), false);
	}
	
	return true;
}

bool CDao::deleteSinger(UINT uSingerID)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_singer SET pos = pos-1 \
			   WHERE pos > (SELECT pos FROM tbl_singer WHERE id = " << uSingerID << L") AND group_id = ( \
					SELECT group_id FROM tbl_singer WHERE id = " << uSingerID << L" \
				)";
	__EnsureReturn(execute(ssSql.str()), false);

	ssSql.str(L"");
	ssSql << L"DELETE FROM tbl_albumitem WHERE id IN( \
					SELECT A.id FROM tbl_albumitem A \
					JOIN tbl_album B ON B.id = A.album_id \
					WHERE B.singer_id = " << uSingerID << L" \
					)";
	ssSql << L"; DELETE FROM tbl_album WHERE singer_id = " << uSingerID;
	ssSql << L"; DELETE FROM tbl_singer WHERE id = " << uSingerID;
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::updateSingerPos(UINT uSingerID, int nPos, int nDstGroupID)
{
	wstringstream ssSql;

	ssSql << L"SELECT group_id FROM tbl_singer WHERE id = " << uSingerID;
	IDBResult *pRst = query(ssSql.str());
	__EnsureReturn(pRst && 1 == pRst->GetRowCount(), false);

	int nSrcGroupID = 0;
	__EnsureReturn(pRst->GetData(0, 0, nSrcGroupID), false);

	ssSql.str(L"");
	ssSql << L"UPDATE tbl_singer SET pos = pos-1 \
			   WHERE pos > (SELECT pos FROM tbl_singer WHERE id = " << uSingerID << L") AND group_id = " << nSrcGroupID;
	__EnsureReturn(execute(ssSql.str()), false);

	ssSql.str(L"");
	ssSql << L"UPDATE tbl_singer SET pos = pos+1 \
			   WHERE pos >= " << nPos << L" AND group_id = " << nDstGroupID;
	__EnsureReturn(execute(ssSql.str()), false);

	ssSql.str(L"");
	ssSql << L"UPDATE tbl_singer SET pos = " << nPos << L", group_id = " << nDstGroupID << L" \
				WHERE id = " << uSingerID;
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

int CDao::addAlbum(const wstring& strName, UINT uSingerID, UINT pos)
{
	CDBTransGuard transGuard(m_db);

	wstringstream ssSql;
	ssSql << L"INSERT INTO tbl_album(name, singer_id, pos) \
			   VALUES(\"" << strName << L"\", " << uSingerID << L", " << pos << L")";
	__EnsureReturn(execute(ssSql.str()), -1);

	int iMaxID = 0;
	__EnsureReturn(_getMaxValue(L"tbl_album", L"id", iMaxID), -1);

	return iMaxID;
}

bool CDao::updateAlbumName(UINT uAlbumID, const wstring& strName)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_album SET name = \"" << strName << L"\" \
				WHERE id = " << uAlbumID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::deleteAlbum(UINT uAlbumID, UINT uSingerID)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_album SET pos = pos-1 \
			   WHERE pos > (SELECT pos FROM tbl_album WHERE id = " << uAlbumID << L") AND singer_id = " << uSingerID;
	ssSql << L"; DELETE FROM tbl_albumitem WHERE album_id = " << uAlbumID;
	ssSql << L"; DELETE FROM tbl_album WHERE id = " << uAlbumID;
	
	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::updateAlbumPos(UINT uAlbumID, int nOldPos, int nNewPos, UINT uSingerID)
{
	int nFlag = nOldPos < nNewPos ? -1 : 1;

	wstringstream ssSql;
	ssSql << L"UPDATE tbl_album SET pos = pos + " << nFlag << L" \
			   WHERE pos BETWEEN " << min(nOldPos, nNewPos) << L" AND " << max(nOldPos, nNewPos) << L" \
			   AND singer_id = " << uSingerID;
	ssSql << L"; UPDATE tbl_album SET pos = " << nNewPos << L" \
					WHERE id = " << uAlbumID << L" \
					AND singer_id = " << uSingerID;
	__EnsureReturn(execute(ssSql.str()), NULL);

	return true;
}

int CDao::_queryMaxAlbumItemPos(int nAlbumID)
{
	wstringstream ssSql;
	ssSql << L"SELECT MAX(pos) FROM tbl_albumitem WHERE album_id = " << nAlbumID;
	IDBResult *pRst = query(ssSql.str());
	__EnsureReturn(pRst, false);

	int nMaxPos = 0;
	(void)pRst->GetData(0, 0, nMaxPos);

	return nMaxPos;
}

bool CDao::addAlbumItem(const list<wstring>& lstOppPaths, UINT uAlbumID, int nPos, const CB_addAlbumItem& cb)
{
	CDBTransGuard transGuard(m_db);

	auto addTime = time(0);

    int nMaxPos = _queryMaxAlbumItemPos(uAlbumID);

	int nAlbumItemID = 0;
	_getMaxValue(L"tbl_albumitem", L"id", nAlbumItemID);

	wstringstream ssSql;

	for (auto& strOppPaths : lstOppPaths)
	{
		ssSql.str(L"");
		ssSql << L"INSERT INTO tbl_albumitem(album_id, path, pos, create_time) \
					VALUES(" << uAlbumID << ", \"" << strOppPaths << "\", " << ++nMaxPos << ", " << addTime << ")";

		ssSql << L"; INSERT INTO tbl_history(path, last_time) VALUES(\"" << strOppPaths << L"\", " << addTime << L")";
		__EnsureReturn(execute(ssSql.str()), NULL);

		cb(++nAlbumItemID, strOppPaths, addTime);
	}

	return true;
}

bool CDao::updateAlbumItemPath(UINT uAlbumItemID, const wstring& strPath)
{
	wstringstream ssSql;
	ssSql << L"UPDATE tbl_albumitem SET path = \"" + strPath + L"\" \
				WHERE id = " << uAlbumItemID;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::deleteAlbumItem(UINT uAlbumItem)
{
	wstringstream ssSql;
	ssSql << L"DELETE FROM tbl_albumitem WHERE id = " << uAlbumItem;

	__EnsureReturn(execute(ssSql.str()), false);

	return true;
}

bool CDao::deleteAlbumItem(const list<UINT>& lstAlbumItemIDs)
{
	wstring strAlbumItemIDList = util::ContainerToStr(lstAlbumItemIDs, L",");

	wstring strSql = L"DELETE FROM tbl_albumitem \
						 WHERE id IN(" + strAlbumItemIDList + L")";
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::setbackAlbumItem(UINT uAlbumID, const list<UINT>& lstAlbumItemID)
{
    int nMaxPos = _queryMaxAlbumItemPos(uAlbumID);

	CDBTransGuard transGuard(m_db);
	wstringstream ssSql;
	for (auto uAlbumItemID : lstAlbumItemID)
	{
		ssSql.str(L"");
		ssSql << L"UPDATE tbl_albumitem SET pos = " << ++nMaxPos << L" WHERE id = " << uAlbumItemID;
		__EnsureReturn(execute(ssSql.str()), false);
	}

	return true;
}

wstring CDao::_getNextName(const wstring& strTableName, const wstring& strNameColumn, const wstring& strBaseName)
{
	wstring strSql
		= L"SELECT IFNULL(MAX(CAST(REPLACE(" + strNameColumn + L", \"" + strBaseName + L"\", '') AS INT)), 0) + 1 \
			FROM " + strTableName + L" \
			WHERE " + strNameColumn + L" like \"" + strBaseName + L"%\"";
	IDBResult *pRst = query(strSql);
	__EnsureReturn(pRst, L"");

	__EnsureReturn(1 == pRst->GetRowCount(), L"");

	int iMaxIndex = 0;
	__EnsureReturn(pRst->GetData(0, 0, iMaxIndex), L"");

	return strBaseName + std::to_wstring(iMaxIndex);
}

wstring CDao::getNewPlaylistName(const wstring& strBaseName)
{
    return _getNextName(L"tbl_playlist", L"name", strBaseName);
}

wstring CDao::getNewSingerGroupName(const wstring& strBaseName)
{
    return _getNextName(L"tbl_singergroup", L"name", strBaseName);
}

wstring CDao::getNewAlbumName(UINT uSingerID, const wstring& strBaseName)
{
	wstring strTable = L"(SELECT * FROM tbl_album WHERE singer_id = " + to_wstring(uSingerID) + L")";
    return _getNextName(strTable, L"name", strBaseName);
}

bool CDao::_getMaxValue(const wstring& strTableName, const wstring& strNameColumn, int& iRet)
{
	wstring strSql = L"SELECT MAX(" + strNameColumn + L") FROM " + strTableName;
	IDBResult *pRst = query(strSql);
	__EnsureReturn(pRst, false);

	__EnsureReturn(pRst->GetData(0, 0, iRet), false);

	return true;
}

bool CDao::queryAlarmmedia(vector<wstring>& vecAlarmmedia)
{
	wstring strSql = L"SELECT path, id FROM tbl_alarmmedia";
	IDBResult *pRst = query(strSql);
	__EnsureReturn(pRst, false);

	wstring strAlarmmedia;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
	{
		__EnsureReturn(pRst->GetData(uRow, 0, strAlarmmedia), false);
		vecAlarmmedia.push_back(strAlarmmedia.c_str());
	}

	return true;
}

bool CDao::addAlarmmedia(const SArray<wstring>& vecAlarmmedia)
{
	wstring strSql;

	for (auto& strAlarmmedia : vecAlarmmedia)
	{
		strSql += L"DELETE FROM tbl_alarmmedia WHERE path = \"" + strAlarmmedia + L"\"; \
							INSERT INTO tbl_alarmmedia(path) values(\"" + strAlarmmedia + L"\");";
	}
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::deleteAlarmmedia(const wstring& strPath)
{
	wstring strSql = L"DELETE FROM tbl_alarmmedia WHERE path = \"" + strPath + L"\"";
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::clearAlarmmedia()
{
	wstring strSql = L"DELETE FROM tbl_alarmmedia";
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::queryPlayRecordMaxTime(time_t& time)
{
	wstringstream ssSql;
	ssSql << L"SELECT max(last_time) FROM tbl_history";

	IDBResult *pRst = query(ssSql.str());
	__EnsureReturn(pRst, false);

	if (0 != pRst->GetRowCount())
	{
		int maxTime = 0;
		__EnsureReturn(pRst->GetData(0, 0, maxTime), false);
		time = (time_t)maxTime;
	}

	return true;
}

bool CDao::queryPlayRecord(time_t time, vector<pair<wstring, int>>& vecPlayRecord)
{
	wstringstream ssSql;
	ssSql << L"SELECT path, last_time \
			   FROM tbl_history \
			   WHERE last_time >= " << time << " \
			   ORDER by last_time";

	IDBResult *pRst = query(ssSql.str());
	__EnsureReturn(pRst, false);

	wstring strPath;
	int nTime = 0;

	UINT uRowCount = pRst->GetRowCount();
	for (UINT uRow = 0; uRow < uRowCount; ++uRow)
	{
		__EnsureReturn(pRst->GetData(uRow, 0, strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, nTime), false);

		vecPlayRecord.push_back({ strPath, nTime });
	}

	return true;
}

bool CDao::clearPlayRecord()
{
	wstring strSql = L"DELETE FROM tbl_history";
	__EnsureReturn(execute(strSql), false);

	return true;
}

bool CDao::queryDifferent(tagCompareBackupResult& result)
{
	wstring strSql = L"SELECT old.path, A.path, B.name \
		FROM src.tbl_playitem old \
		JOIN tbl_playitem A ON A.id = old.id AND A.create_time = old.create_time \
		JOIN tbl_playlist B on B.id = A.playlist_id \
		WHERE A.path <> old.path \
		ORDER BY B.pos, A.pos";
	IDBResult* pRst = query(strSql);
	__EnsureReturn(pRst, false);

	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagModifyedMedia ModifyedMedia;
		__EnsureReturn(pRst->GetData(uRow, 0, ModifyedMedia.strOldPath), false);

		__EnsureReturn(pRst->GetData(uRow, 1, ModifyedMedia.strNew), false);
		if (fsutil::GetParentDir(ModifyedMedia.strNew) == fsutil::GetParentDir(ModifyedMedia.strOldPath))
		{
			ModifyedMedia.strNew = fsutil::GetFileName(ModifyedMedia.strNew);
		}

		__EnsureReturn(pRst->GetData(uRow, 2, ModifyedMedia.strMediaSetName), false);
		result.arrModifyedMedia.add(ModifyedMedia);
	}

	strSql = L"SELECT old.path, A.path, B.name, C.name, C.path \
		FROM src.tbl_albumitem old \
		JOIN tbl_albumitem A ON A.id = old.id AND IFNULL(A.create_time,0) = IFNULL(old.create_time,0) \
		JOIN tbl_album B ON B.id = A.album_id \
		JOIN tbl_singer C ON C.id = B.singer_id \
		JOIN tbl_singergroup D ON D.id = C.group_id \
		WHERE A.path <> old.path \
		ORDER BY LOWER(D.name), C.pos, B.pos, A.pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);
	wstring strSingerDir;
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagModifyedMedia ModifyedMedia;
		__EnsureReturn(pRst->GetData(uRow, 0, ModifyedMedia.strOldPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, ModifyedMedia.strNew), false);
		__EnsureReturn(pRst->GetData(uRow, 2, ModifyedMedia.strMediaSetName), false);
		__EnsureReturn(pRst->GetData(uRow, 3, ModifyedMedia.strSingerName), false);
		__EnsureReturn(pRst->GetData(uRow, 4, strSingerDir), false);

		if (fsutil::GetParentDir(ModifyedMedia.strNew) == fsutil::GetParentDir(ModifyedMedia.strOldPath))
		{
			ModifyedMedia.strNew = fsutil::GetFileName(ModifyedMedia.strNew);
		}
		else
		{
			ModifyedMedia.strNew = strSingerDir + ModifyedMedia.strNew;
		}

		ModifyedMedia.strOldPath = strSingerDir + ModifyedMedia.strOldPath;

		result.arrModifyedMedia.add(ModifyedMedia);
	}

	strSql = L"SELECT A.path, B.name FROM src.tbl_playitem A \
		JOIN src.tbl_playlist B on B.id = A.playlist_id AND B.id <> 0 \
		WHERE NOT EXISTS(SELECT id FROM tbl_playitem new \
			where new.id == A.id AND new.create_time = A.create_time) \
		ORDER BY B.pos, A.pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagDeletedPlayItem DeletedPlayItem;
		__EnsureReturn(pRst->GetData(uRow, 0, DeletedPlayItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, DeletedPlayItem.strPlaylistName), false);
		result.arrDeletedPlayItem.add(DeletedPlayItem);
	}

	strSql = L"SELECT A.path, B.name, C.name, C.path, C.id \
		FROM src.tbl_albumitem A \
		JOIN src.tbl_album B ON B.id = A.album_id \
		JOIN src.tbl_singer C ON C.id = B.singer_id \
		JOIN src.tbl_singergroup D ON D.id = C.group_id \
		WHERE NOT EXISTS(SELECT id FROM tbl_albumitem new WHERE new.id == A.id \
			AND IFNULL(new.create_time,0) = IFNULL(A.create_time,0)) \
		ORDER BY LOWER(D.name), C.pos, B.pos, A.pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagDeletedAlbumItem DeletedAlbumItem;
		__EnsureReturn(pRst->GetData(uRow, 0, DeletedAlbumItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, DeletedAlbumItem.strAlbumName), false);
		__EnsureReturn(pRst->GetData(uRow, 2, DeletedAlbumItem.strSingerName), false);
		__EnsureReturn(pRst->GetData(uRow, 3, DeletedAlbumItem.strSingerDir), false);
		__EnsureReturn(pRst->GetData(uRow, 4, DeletedAlbumItem.iSingerID), false);
		result.arrDeletedAlbumItem.add(DeletedAlbumItem);
	}

	strSql = L"SELECT A.path, B.name FROM tbl_playitem A \
		JOIN tbl_playlist B on B.id = A.playlist_id \
		WHERE NOT EXISTS(SELECT id FROM src.tbl_playitem old \
			WHERE old.id = A.id AND old.create_time = A.create_time) \
		ORDER BY B.pos, A.pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagNewPlayItem NewPlayItem;
		__EnsureReturn(pRst->GetData(uRow, 0, NewPlayItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, NewPlayItem.strPlaylistName), false);
		result.arrNewPlayItem.add(NewPlayItem);
	}

	strSql = L"SELECT A.path, B.name, C.name, C.path, c.id \
		FROM tbl_albumitem A \
		JOIN tbl_album B ON B.id = A.album_id \
		JOIN tbl_singer C ON C.id = B.singer_id \
		JOIN tbl_singergroup D ON D.id = C.group_id \
		WHERE NOT EXISTS(SELECT id FROM src.tbl_albumitem old WHERE old.id = A.id \
			AND IFNULL(old.create_time, 0) = IFNULL(A.create_time, 0)) \
		ORDER BY LOWER(D.name), C.pos, B.pos, A.pos";
	pRst = query(strSql);
	__EnsureReturn(pRst, false);
	for (UINT uRow = 0; uRow < pRst->GetRowCount(); uRow++)
	{
		tagNewAlbumItem NewAlbumItem;
		__EnsureReturn(pRst->GetData(uRow, 0, NewAlbumItem.strPath), false);
		__EnsureReturn(pRst->GetData(uRow, 1, NewAlbumItem.strAlbumName), false);
		__EnsureReturn(pRst->GetData(uRow, 2, NewAlbumItem.strSingerName), false);
		__EnsureReturn(pRst->GetData(uRow, 3, NewAlbumItem.strSingerDir), false);
		__EnsureReturn(pRst->GetData(uRow, 4, NewAlbumItem.iSingerID), false);
		result.arrNewAlbumItem.add(NewAlbumItem);
	}

	return true;
}

bool CDao::updateFiles(const wstring& strOldOppPath, const wstring& strNewOppPath, bool bDir)
{
	for (const wstring& strTable : { L"tbl_alarmmedia", L"tbl_history" })
	{
		wstring strSql;
		if (!bDir)
		{
			strSql = L"UPDATE " + strTable + L" SET path = \"" + strNewOppPath + L"\" \
							WHERE path = \"" + strOldOppPath + L"\";";

			__EnsureReturn(execute(strSql.c_str()), false);
		}
		else
		{
			strSql = L"UPDATE " + strTable + L" SET path = \"" + strNewOppPath
				+ L"\" || SUBSTR(path, LENGTH(\"" + strOldOppPath + L"\")+1) \
						WHERE path LIKE \"" + strOldOppPath + L"\\%\";";

			__EnsureReturn(execute(strSql.c_str()), false);
		}
	}

	return true;
}

bool CDao::deleteFiles(const set<wstring>& setDeleteFiles)
{
	wstring strDeletedPathList = util::ContainerToStr(setDeleteFiles, L"\",\"");
	strDeletedPathList = util::StrLowerCase(L"\"" + strDeletedPathList + L"\"");

	wstring strSql;

	for (const wstring& strTable : { L"tbl_alarmmedia", L"tbl_history" })
	{
		strSql = L"DELETE FROM " + strTable + L" WHERE LOWER(path) IN(" + strDeletedPathList + L")";
		__EnsureReturn(execute(strSql), false);
	}

	return true;
}

bool CDao::clearAll()
{
	wstring lpTables[] = {
		L"tbl_playlist"
		, L"tbl_playitem"

		, L"tbl_singergroup"
		, L"tbl_singer"
		, L"tbl_album"
		, L"tbl_albumitem"

		, L"tbl_history"

		, L"tbl_alarmmedia"
	};

	wstring strSql;
	for (UINT uIndex = 0; uIndex < sizeof(lpTables) / sizeof(lpTables[0]); ++uIndex)
	{
		strSql += L"DELETE FROM " + lpTables[uIndex] + L";";
	}

	return execute(strSql);
}

//
//bool CDao::findSinger(const wstring& strPath, map<wstring, pair<UINT, wstring>>& mapSingerInfo)
//{
//	wstringstream ssSql;
//	ssSql << "SELECT id, name, substr(path, " << strPath.size() + 2 << ") as dir FROM tbl_singer \
//		 where path like '" << strPath << "\\%' and path not like '" << strPath << "\\%\\%'";
//
//	IDBResult *pRst = query(ssSql.str());
//	__EnsureReturn(pRst, false);
//
//	int iSingerID = 0;
//	wstring strSingerName;
//	wstring strSingerDir;
//	for (UINT uRow = 0; uRow < pRst->GetRowCount(); ++uRow)
//	{
//		__EnsureReturn(pRst->GetData(uRow, 0, iSingerID), false);
//		__EnsureReturn(pRst->GetData(uRow, 1, strSingerName), false);
//		__EnsureReturn(pRst->GetData(uRow, 2, strSingerDir), false);
//
//		mapSingerInfo[strSingerDir] = { iSingerID, strSingerName };
//	}
//
//	return true;
//}
