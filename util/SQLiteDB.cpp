
#include "util.h"

#include "SQLiteDB.h"

#include "sqlite/sqlite3.h"


//CSQLiteDBResult

CSQLiteDBResult::~CSQLiteDBResult()
{
	if (m_pData)
	{
		sqlite3_free_table(m_pData);
    }
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, string& strData)
{
	__EnsureReturn(uRow < m_uRowCount && uColumn < m_uColumnCount, false);
	__EnsureReturn(m_pData, false);

	char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
	if (NULL != lpData)
	{
		strData = lpData;
	}

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, wstring& strData)
{
	string t_strData;
	__EnsureReturn(GetData(uRow, uColumn, t_strData), false);
	
	strData = util::StrToWStr(t_strData, CP_UTF8);
	
	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, int& nValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	nValue = atoi(strData.c_str());

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, double& dbValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	dbValue = atof(strData.c_str());

	return true;
}

bool CSQLiteDBResult::_getData(UINT uRow, const function<void(const string&)>& cb)
{
	__EnsureReturn(uRow < m_uRowCount, false);
	__EnsureReturn(m_pData, false);

	for (UINT uColumn = 0; uColumn < m_uColumnCount; uColumn++)
	{
		char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
		if (NULL != lpData)
		{
			cb(lpData);
		}
		else
		{
			cb("");
		}
	}

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<string>& arrData)
{
	return _getData(uRow, [&](const string& strData) {
		arrData.add(strData);
	});
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<wstring>& arrData)
{
	return _getData(uRow, [&](const string& strData) {
		arrData.add(util::StrToWStr(strData, CP_UTF8));
	});
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<int>& arrValue)
{
	return _getData(uRow, [&](const string& strData) {
		arrValue.add(atoi(strData.c_str()));
	});
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<double>& arrValue)
{
	return _getData(uRow, [&](const string& strData) {
		arrValue.add(atof(strData.c_str()));
	});
}


//CSQLiteDB

CSQLiteDB::CSQLiteDB(const string& strDBPath)
	: m_strDBPath(strDBPath)
{
}

CSQLiteDB::~CSQLiteDB()
{
	Disconnect();
}

int CSQLiteDB::GetStatus()
{
	return (NULL != m_hDB);
}

bool CSQLiteDB::Connect(const string& strPara)
{
	__EnsureReturn(!m_hDB, false);

	string strDBPath = !strPara.empty()?strPara:m_strDBPath;
	
	__EnsureReturn(SQLITE_OK == sqlite3_open(strDBPath.c_str(), (sqlite3**)&m_hDB), false);
	__EnsureReturn(m_hDB, false);

	return true;
}

bool CSQLiteDB::Disconnect()
{
	__EnsureReturn(m_hDB, false);

	__EnsureReturn(SQLITE_OK == sqlite3_close((sqlite3*)m_hDB), false);

	m_hDB = NULL;

	return true;
}

bool CSQLiteDB::Execute(const string& strSql)
{
	__EnsureReturn(m_hDB, false);

	char *pszError = NULL;
	int iRet = sqlite3_exec((sqlite3*)m_hDB, strSql.c_str(), 0, 0, &pszError);
	if (NULL != pszError)
	{
		m_strError = pszError;
	}
	else
	{
		m_strError.clear();
	}

	return SQLITE_OK == iRet;
}

bool CSQLiteDB::Execute(const wstring& strSql)
{
	return Execute(util::WStrToStr(strSql, CP_UTF8));
}

IDBResult* CSQLiteDB::Query(const string& strSql)
{
	__EnsureReturn(m_hDB, NULL);

	char ** pData = NULL;

	int nColumnCount = 0;
	int nRowCount = 0;

	char *pszError = NULL;
	int iRet = sqlite3_get_table((sqlite3*)m_hDB, strSql.c_str(), &pData
		, &nRowCount, &nColumnCount, &pszError);
	if (pszError)
	{
		m_strError = pszError;
	}
	else
	{
		m_strError.clear();
	}

	__EnsureReturn(SQLITE_OK == iRet && pData, NULL);
	
	CSQLiteDBResult* pSQLiteDBResult = new CSQLiteDBResult;

	pSQLiteDBResult->m_uColumnCount = (UINT)nColumnCount;
	pSQLiteDBResult->m_uRowCount = (UINT)nRowCount;

	pSQLiteDBResult->m_pData = pData;

	return pSQLiteDBResult;
}

IDBResult* CSQLiteDB::Query(const wstring& strSql)
{
	return Query(util::WStrToStr(strSql, CP_UTF8));
}

bool CSQLiteDB::BeginTrans()
{
	__EnsureReturn(!m_bInTrans, false);
	
	__EnsureReturn(Execute(L"begin Transaction"), false);

	m_bInTrans = true;

	return true;
}

bool CSQLiteDB::CommitTrans()
{
	if (m_bInTrans)
	{
		__EnsureReturn(Execute(L"commit Transaction"), false);

		m_bInTrans = false;
	}

	return true;
}

bool CSQLiteDB::RollbakTrans()
{
	if (m_bInTrans)
	{
		__EnsureReturn(Execute(L"rollback Transaction"), false);

		m_bInTrans = false;
	}

	return true;
}
