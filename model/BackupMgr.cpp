
#include <model.h>

#include "DBCompare.h" 

void CBackupMgr::init()
{
	wchar_t pszCurrDir[MAX_PATH];
	memset(pszCurrDir, 0, sizeof pszCurrDir);
	::GetCurrentDirectoryW(sizeof(pszCurrDir), pszCurrDir);
	m_strBackupDir = pszCurrDir;
	m_strBackupDir.append(L"\\bak\\");

	fsutil_win::FindFile(m_strBackupDir + L"bak_*", [&](const tagFindData& findData) {
		m_arrBackupFile.add(findData.getFileName());
		return true;
	});
	
	CDBCompare::inst().init(m_strBackupDir, std::bind(&CBackupMgr::_onCompareFinish, this, std::placeholders::_1));
}

void CBackupMgr::close()
{
	CDBCompare::inst().stop();

	m_arrBackupFile.clear();

	m_mapCompareResult.clear();

	m_cb = NULL;
}

wstring CBackupMgr::backup()
{
	wchar_t pszBakFileName[64];
	memset(pszBakFileName, 0, sizeof pszBakFileName);

	SYSTEMTIME sysTime;
	util::getCurrentTime(sysTime);
	wsprintf(pszBakFileName, L"bak_%u.%02u.%02u_%02u.%02u.%02u"
		, sysTime.wYear, sysTime.wMonth, sysTime.wDay
		, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	wstring strBakFile = m_strBackupDir + pszBakFileName;

	wstring strDBFile = m_strBackupDir + L"../data.db";
	if (!::CopyFile(strDBFile.c_str(), strBakFile.c_str(), TRUE))
	{
		return L"";
	}

	m_arrBackupFile.add(pszBakFileName);

	return pszBakFileName;
}

void CBackupMgr::removeBackup(const wstring& strFile)
{
	CDBCompare::inst().cancelTask(strFile);

	wstring strFilePath = m_strBackupDir + strFile;
	::DeleteFile(strFilePath.c_str());
	
	m_arrBackupFile.del(strFile);

	std::lock_guard<mutex> guard(m_mtx);

	m_mapCompareResult.del(strFile);
	m_mapCompareResult([&](TD_CompareResultMap& mapCompareResult){
		mapCompareResult.del(strFile);
	});
}

void CBackupMgr::getCompareResult(const SList<pair<wstring, wstring>>& lstFiles, SList<tagCompareBackupResult>& lstResult)
{
	std::lock_guard<mutex> guard(m_mtx);

	for (auto& pr : lstFiles)
	{
		m_mapCompareResult.get(pr.first, [&](TD_CompareResultMap& mapCompareResult) {
			mapCompareResult.get(pr.second, [&](tagCompareBackupResult& result) {
				lstResult.add(result);
			});
		});
	};
}

bool CBackupMgr::_getCompareResult(const wstring& strSrcFile, const wstring& strDstFile, const CB_BackupCompare& cb)
{
	m_mtx.lock();

	bool bRet = false;
	m_mapCompareResult.get(strSrcFile, [&](TD_CompareResultMap& mapCompareResult) {
		mapCompareResult.get(strDstFile, [&](tagCompareBackupResult& result) {
			m_mtx.unlock();

			cb(result);

			bRet = true;
		});
	});

	if (!bRet)
	{
		m_mtx.unlock();
	}

	return bRet;
}

void CBackupMgr::_saveCompareResult(const tagCompareBackupResult& result)
{
	CB_BackupCompare cb;

	m_mtx.lock();

	if (!m_mapCompareResult.get(result.strSrcFile, [&](TD_CompareResultMap& mapCompareResult) {
		mapCompareResult.set(result.strDstFile, result);
	}))
	{
		m_mapCompareResult.insert(result.strSrcFile).set(result.strDstFile, result);
	}

	if (m_cb && m_prWaitTask.first == result.strSrcFile && m_prWaitTask.second == result.strDstFile)
	{
		cb = m_cb;
		m_cb = NULL;
	}
	
	m_mtx.unlock();

	if (cb)
	{
		cb(result);
	}
}

void CBackupMgr::compareBackup(const SList<pair<wstring, wstring>>& lstFiles, SList<tagCompareBackupResult>& lstResult)
{
	SList<pair<wstring, wstring>> lstTask;
	for (auto& pr : lstFiles)
	{
        if (!_getCompareResult(pr.first, pr.second, [&](const tagCompareBackupResult& result) {
			lstResult.add(result);
		}))
		{
			lstTask.add(pr);
		}
	}

	if (lstTask)
	{
		CDBCompare::inst().addTask(lstTask);
	}
}

void CBackupMgr::compareBackup(const wstring& strSrcFile, const wstring& strDstFile, const CB_BackupCompare& cb)
{
    if (!_getCompareResult(strSrcFile, strDstFile, [&](const tagCompareBackupResult& result) {
		if (cb)
		{
			cb(result);
		}
	}))
	{
		if (cb)
		{
			std::lock_guard<mutex> guard(m_mtx);

			m_cb = cb;
			m_prWaitTask = { strSrcFile, strDstFile };
		}

		SList<pair<wstring, wstring>> lstTask{ { strSrcFile, strDstFile } };
		CDBCompare::inst().addTask(lstTask);
	}
}

bool CBackupMgr::compareBackup(const wstring& strSrcFile, tagCompareBackupResult& result)
{
	result.strSrcFile = strSrcFile;
	__EnsureReturn(CDBCompare::inst().compareBackup(m_dao, result), false);
	
	_saveCompareResult(result);
	
	return true;
}

void CBackupMgr::_onCompareFinish(const tagCompareBackupResult& result)
{
	_saveCompareResult(result);
}
