
#include <model.h>

#include "DBCompare.h"

void CDBCompare::init(const wstring& strBackupDir, const CB_BackupCompare& cb)
{
	m_strBackupDir = strBackupDir;
	m_cb = cb;
}

void CDBCompare::_threadLoop()
{
	while (true)
	{
		this_thread::yield();
		//this_thread::sleep_for(std::chrono::milliseconds(500));

		wstring strSrcFile;
		wstring strDstFile;
		if (!_getTask(strSrcFile, strDstFile))
		{
			break;
		}

		CSQLiteDB db;
		CDao dao(db);

		string strDstPath = util::WStrToStr(m_strBackupDir + strDstFile);

		tagCompareBackupResult result(strSrcFile, strDstFile);

		bool bRet = false;

		_lockFile(strDstFile);
		if (db.Connect(strDstPath))
		{
			bRet = _queryDifferent(dao, result);
			(void)db.Disconnect();
		}
		_unlockFile(strDstFile);

		if (bRet)
		{
			_analysisDeleted(result);

			m_cb(result);
		}
	}
}

void CDBCompare::addTask(const SList<pair<wstring, wstring>>& lstTask)
{
	std::lock_guard<mutex> guard(m_mtx);

	m_plTask.del(lstTask);
	m_plTask.add(lstTask);

	if (!m_thr.joinable())
	{
		m_thr = thread([&] {
			this->_threadLoop();
			m_thr.detach();
		});
	}
}

bool CDBCompare::_getTask(wstring& strSrcFile, wstring& strDstFile)
{
	std::lock_guard<mutex> guard(m_mtx);

    return m_plTask.popBack([&](pair<wstring, wstring>& pr) {
		strSrcFile = pr.first;
		strDstFile = pr.second;
	});
}

void CDBCompare::cancelTask(const wstring& strFile)
{
	std::unique_lock<mutex> lock(m_mtx);

    m_plTask.del_ex([&](pair<wstring, wstring>& pr) {
		return pr.first == strFile || pr.second == strFile;
	});
}

void CDBCompare::clearTask()
{
	std::lock_guard<mutex> guard(m_mtxLockFile);

	m_plTask.clear();
}

void CDBCompare::stop()
{
	clearTask();

	if (m_thr.joinable())
	{
		m_thr.join();
	}
}

bool CDBCompare::_queryDifferent(CDao& dao, tagCompareBackupResult& result)
{
	auto& strSrcFile = result.strSrcFile;

	wstring strSql = L"attach \"" + m_strBackupDir + strSrcFile + L"\" as src";

	_lockFile(strSrcFile);
	__EnsureReturn(dao.execute(strSql), false);

	//CDaoTransGuard transGuard(dao);
	bool bRet = dao.queryDifferent(result);
	//transGuard.Commit();

	strSql = L"detach src";
	(void)dao.execute(strSql);
	_unlockFile(strSrcFile);

	return bRet;
}

void CDBCompare::_analysisDeleted(tagCompareBackupResult& result)
{
	result.arrDeletedPlayItem.del_ex([&](tagDeletedPlayItem& DeletedPlayItem) {
		if (!result.arrNewPlayItem)
		{
			return E_DelConfirm::DC_Abort;
		}

		if (0 != result.arrNewPlayItem.del_ex([&](tagNewPlayItem& NewPlayItem) {
			if (NewPlayItem.strPath == DeletedPlayItem.strPath)
			{
				if (NewPlayItem.strPlaylistName != DeletedPlayItem.strPlaylistName)
				{
					tagMovedMedia MovedMedia;
					MovedMedia.strPath = DeletedPlayItem.strPath;
					MovedMedia.strOldMediaSetName = DeletedPlayItem.strPlaylistName;
					MovedMedia.strNewMediaSetName = NewPlayItem.strPlaylistName;
					result.arrMovedMedia.add(MovedMedia);
				}

				return E_DelConfirm::DC_YesAbort;
			}

			return E_DelConfirm::DC_No;
		}))
		{
			return E_DelConfirm::DC_Yes;
		}

		return E_DelConfirm::DC_No;
	});

	result.arrDeletedAlbumItem.del_ex([&](tagDeletedAlbumItem& DeletedAlbumItem) {
		if (!result.arrNewAlbumItem)
		{
			return E_DelConfirm::DC_Abort;
		}

		if (0 != result.arrNewAlbumItem.del_ex([&](tagNewAlbumItem& NewAlbumItem) {
			if (NewAlbumItem.iSingerID == DeletedAlbumItem.iSingerID
				&& NewAlbumItem.strPath == DeletedAlbumItem.strPath)
			{
				if (NewAlbumItem.strAlbumName != DeletedAlbumItem.strAlbumName)
				{
					tagMovedMedia MovedMedia;
					MovedMedia.strPath = NewAlbumItem.strSingerDir + NewAlbumItem.strPath;
					MovedMedia.strOldMediaSetName = DeletedAlbumItem.strAlbumName;
					MovedMedia.strNewMediaSetName = NewAlbumItem.strAlbumName;
					MovedMedia.strSingerName = NewAlbumItem.strSingerName;
					result.arrMovedMedia.add(MovedMedia);
				}

				return E_DelConfirm::DC_YesAbort;
			}

			return E_DelConfirm::DC_No;
		}))
		{
			return E_DelConfirm::DC_Yes;
		}

		return E_DelConfirm::DC_No;
	});
}

bool CDBCompare::compareBackup(CDao& dao, tagCompareBackupResult& result)
{
	__EnsureReturn(_queryDifferent(dao, result), false);

	_analysisDeleted(result);

	return true;
}

bool CDBCompare::_checkLockedFile(const wstring& strFile, const function<void(wstring& strLockedFile)>& cb)
{
	for (auto& strLockedFile : m_lpLockedFile)
	{
		if (strLockedFile == strFile)
		{
			if (cb)
			{
				cb(strLockedFile);
			}
			
			return true;
		}
	}

	return false;
}

bool CDBCompare::_lockFile(const wstring& strFile)
{
	m_mtxLockFile.lock();

	if (!_checkLockedFile(strFile))
	{
		bool bRet = _checkLockedFile(L"", [&](wstring& strLockedFile) {
			strLockedFile = strFile;
		});

		m_mtxLockFile.unlock();

		return bRet;
	}

	m_mtxLockFile.unlock();

	std::unique_lock<mutex> lock(m_mtxLockFile);
	m_sgnLockFile.wait(lock);
	
	return _checkLockedFile(L"", [&](wstring& strLockedFile) {
		strLockedFile = strFile;
	});
}

bool CDBCompare::_unlockFile(const wstring& strFile)
{
	std::lock_guard<mutex> guard(m_mtxLockFile);

	return _checkLockedFile(strFile, [&](wstring& strLockedFile) {
		strLockedFile.clear();
		m_sgnLockFile.notify_one();
	});
}
