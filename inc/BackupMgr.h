#pragma once

using CB_BackupCompare = function<void(const tagCompareBackupResult&)>;

class __ModelExt CBackupMgr
{
public:
	CBackupMgr(CDao& dao)
		: m_dao(dao)
	{
	}

private:
	CDao& m_dao;
	
	wstring m_strBackupDir;

	SArray<wstring> m_arrBackupFile;

	std::mutex m_mtx;
	using TD_CompareResultMap = SHashMap<wstring, tagCompareBackupResult>;
	SHashMap<wstring, TD_CompareResultMap> m_mapCompareResult;

	CB_BackupCompare m_cb;
	pair<wstring, wstring> m_prWaitTask;

private:
	void _onCompareFinish(const tagCompareBackupResult& result);

	void _saveCompareResult(const tagCompareBackupResult& result);

	bool _getCompareResult(const wstring& strSrcFile, const wstring& strDstFile, const CB_BackupCompare& cb);

public:
	const wstring& getBackupDir()
	{
		return m_strBackupDir;
	}

	const SArray<wstring>& getBackupFiles()
	{
		return m_arrBackupFile;
	}

	void init();

	void close();

	wstring backup();

	void removeBackup(const wstring& strFile);
	
	void getCompareResult(const SList<pair<wstring, wstring>>& lstFiles, SList<tagCompareBackupResult>& lstResult);

	void compareBackup(const SList<pair<wstring, wstring>>& lstFiles, SList<tagCompareBackupResult>& lstResult);

	void compareBackup(const wstring& strSrcFile, const wstring& strDstFile, const CB_BackupCompare& cb=NULL);

	bool compareBackup(const wstring& strSrcFile, tagCompareBackupResult& result);
};
