#pragma once

class CDBCompare
{
private:
	CDBCompare() {}

public:
	static CDBCompare& inst()
	{
		static CDBCompare inst;
		return inst;
	}

private:
	wstring m_strBackupDir;
	
	CB_BackupCompare m_cb;

	thread m_thr;
	std::mutex m_mtx;

	PairList<wstring, wstring> m_plTask;

	wstring m_lpLockedFile[3];

	std::mutex m_mtxLockFile;
	std::condition_variable_any m_sgnLockFile;

private:
	void _threadLoop();

	bool _getTask(wstring& strSrcFile, wstring& strDstFile);

	bool _queryDifferent(CDao& dao, tagCompareBackupResult& result);
	void _analysisDeleted(tagCompareBackupResult& result);

	bool _checkLockedFile(const wstring& strFile, const function<void(wstring& strLockedFile)>& cb = NULL);
	bool _lockFile(const wstring& strFile);
	bool _unlockFile(const wstring& strFile);

public:
	void init(const wstring& strBackupDir, const CB_BackupCompare& cb);
	
	void addTask(const SList<pair<wstring, wstring>>& lstTask);

	void cancelTask(const wstring& strFile);

	void clearTask();

	void stop();

	bool compareBackup(CDao& dao, tagCompareBackupResult& result);
};
