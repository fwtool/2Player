
#include <MediaDef.h>

const tagTrackInfo tagTrackInfo::NoTrack;
const CCueFile CCueFile::NoCue;

const unsigned short g_utf8bom = 0xFEFF;

bool CCueFile::updateTitle(const wstring& strTitle)
{
	m_strTitle = strTitle;

	m_vecLineData.set(m_uLineTitle, "TITLE " + util::WStrToStr(strTitle));

	return true;
}

bool CCueFile::updateMediaFileName(const wstring& strMediaFileName)
{
	m_strMediaFileName = strMediaFileName;

	return true;
}

CRTrackInfo CCueFile::getTrack(UINT uMs) const
{
	auto pRet = &tagTrackInfo::NoTrack;

	for (auto& trackInfo : m_alTrackInfo)
	{
		if (uMs >= trackInfo.uMsBegin)
		{
			pRet = &trackInfo;
		}
	}

	return *pRet;
}

bool CCueFile::load(const wstring& strFile)
{
    if (!fsutil::loadTxt(strFile, m_vecLineData))
	{
		return false;
	}

	m_vecLineData.getFront([](string& strLineData) {
		if (-17 == strLineData[0] && -69 == strLineData[1] && -65 == strLineData[2])
		{
			strLineData[0] = ' ';
			strLineData[1] = ' ';
			strLineData[2] = ' ';
		}
	});

	bool bTrack = false;
    m_vecLineData([&](string& strLineData, UINT uIdx) {
		SVector<wstring> vecCueLine;

		strLineData.erase(0, strLineData.find_first_not_of(' '));

		while (!strLineData.empty())
		{
			size_t startPos = 0;
			size_t pos = 0;
			if (strLineData.front() == '"')
			{
				startPos = 1;
				pos = strLineData.find('"', 1);
				if (wstring::npos == pos)
				{
					return false;
				}
			}
			else
			{
				pos = strLineData.find(' ');
				if (wstring::npos == pos)
				{
					vecCueLine.add(util::StrToWStr(strLineData, util::IsUTF8Str(strLineData) ? CP_UTF8 : CP_ACP));

					break;
				}
			}

			string strSub = strLineData.substr(startPos, pos - startPos);
			vecCueLine.add(util::StrToWStr(strSub, util::IsUTF8Str(strSub) ? CP_UTF8 : CP_ACP));

			strLineData = strLineData.substr(pos + 1);
		}

        (void)vecCueLine.getFront([&](wstring& str) {
			if (L"PERFORMER" == str)
			{
				if (!bTrack)
				{
					(void)vecCueLine.get(1, m_strPerformer);
				}
				else
				{
					m_alTrackInfo.getBack([&](tagTrackInfo& TrackInfo) {
						(void)vecCueLine.get(1, TrackInfo.strPerformer);
					});
				}
			}
			else if (L"TITLE" == str)
			{
				if (!bTrack)
				{
					(void)vecCueLine.get(1, m_strTitle);
					m_uLineTitle = uIdx;
				}
				else
				{
					m_alTrackInfo.getBack([&](tagTrackInfo& TrackInfo) {
						(void)vecCueLine.get(1, TrackInfo.strTitle);
					});
				}
			}
			else if (L"FILE" == str)
			{
				(void)vecCueLine.get(1, m_strMediaFileName);
				m_uLineMediaFileName = uIdx;
			}
			else if (L"TRACK" == str)
			{
				bTrack = true;

				tagTrackInfo trackInfo;
				trackInfo.uIndex = m_alTrackInfo.size();
				m_alTrackInfo.add(trackInfo);
			}
			else if (L"INDEX" == str)
			{
				m_alTrackInfo.getBack([&](tagTrackInfo& TrackInfo) {
					wstring strPos;
					(void)vecCueLine.get(2, strPos);

					UINT uMin = 0;
					UINT uSec = 0;
					UINT uXs = 0;
                    (void)swscanf(strPos.c_str(), L"%u:%u:%u", &uMin, &uSec, &uXs);
                    TrackInfo.uMsBegin = (uMin * 60 + uSec)*1000 + uXs * 10;
				});
			}
		});

		return true;
	});

	return true;
}
