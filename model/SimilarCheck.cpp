
#include <model.h>

#include "SimilarCheck.h"

using TD_CheckSimilarMultiTask = NS_mtutil::CMultiTask<pair<CMediaRes*, tagMediaResInfo>, TD_SimilarGroupMap>;
using TD_CheckSimilarSubTask = TD_CheckSimilarMultiTask::CB_SubTask;

static void _checkSimilarFile(PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo, TD_SimilarFile& arrResult, const TD_CheckSimilarSubTask& cb)
{
	UINT uThreadCount = plMediaResInfo.size() / 1000;
    uThreadCount = MIN(4, uThreadCount);

	TD_CheckSimilarMultiTask MultiTask;
	auto& vecSimilarGroup = MultiTask.start(plMediaResInfo, uThreadCount, cb);

	TD_SimilarGroupMap mapRet;
	for (auto& mapSimilarGroup : vecSimilarGroup)
	{
		mapRet.add(mapSimilarGroup);
	}
	mapRet([&](TD_SimilarFileGroup& arrSimilarGroup) {
		arrResult.add(arrSimilarGroup);
	});
}

void CSimilarCheck::checkSimilarFile(PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo
	, CB_checkSimilarFile cb, TD_SimilarFile& arrResult)
{
	UINT uMaxIndex = plMediaResInfo.size() - 2;

    _checkSimilarFile(plMediaResInfo, arrResult, [&](UINT taskIdx, pair<CMediaRes*, tagMediaResInfo>& task, TD_SimilarGroupMap& mapSimilarGroup) {
		if (taskIdx <= uMaxIndex)
		{
			auto pMediaRes = task.first;
			if (!cb(*pMediaRes))
			{
				return false;
			}

			auto& MediaResInfo1 = task.second;

			TD_SimilarFileGroup arrSimilarGroup;

            plMediaResInfo(taskIdx + 1, [&](pair<CMediaRes*, tagMediaResInfo>& pr) {
				auto& MediaResInfo2 = pr.second;

				UINT uPercent = compareMediaResInfo(MediaResInfo1, MediaResInfo2);
				if (0 != uPercent)
				{
					arrSimilarGroup.add({ pr.first, uPercent });
				}
			});

			if (arrSimilarGroup)
			{
				mapSimilarGroup.insert(taskIdx, arrSimilarGroup).addFront({ pMediaRes,100 });
			}
		}

		return true;
	});

}

void CSimilarCheck::checkSimilarFile(PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo1
	, PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo2
	, CB_checkSimilarFile cb, TD_SimilarFile& arrResult)
{
    _checkSimilarFile(plMediaResInfo1, arrResult, [&](UINT taskIdx, pair<CMediaRes*, tagMediaResInfo>& task, TD_SimilarGroupMap& mapSimilarGroup) {
		auto pMediaRes = task.first;
		if (!cb(*pMediaRes))
		{
			return false;
		}

		auto& MediaResInfo1 = task.second;

		TD_SimilarFileGroup arrSimilarGroup;

        plMediaResInfo2([&](pair<CMediaRes*, tagMediaResInfo>& pr) {
			auto& MediaResInfo2 = pr.second;

			UINT uPercent = compareMediaResInfo(MediaResInfo1, MediaResInfo2);
			if (0 != uPercent)
			{
				arrSimilarGroup.add({ pr.first, uPercent });
			}
		});

		if (arrSimilarGroup)
		{
			mapSimilarGroup.insert(taskIdx, arrSimilarGroup).addFront({ pMediaRes,100 });
		}

		return true;
	});
}

UINT CSimilarCheck::compareMediaResInfo(const tagMediaResInfo& MediaResInfo1, const tagMediaResInfo& MediaResInfo2)
{
	if (MediaResInfo1.FileTitle.setFileTitle.size() > 1 && MediaResInfo2.FileTitle.setFileTitle.size() > 1)
	{
		return compareTitle(MediaResInfo1.FileTitle.setFileTitle, MediaResInfo2.FileTitle.setFileTitle);
	}
	else if (MediaResInfo1.FileTitle.setFileTitle.size() > 1)
	{
		return compareTitle(MediaResInfo2.FileTitle.strFileTitle, MediaResInfo1.FileTitle.setFileTitle);
	}
	else if (MediaResInfo2.FileTitle.setFileTitle.size() > 1)
	{
		return compareTitle(MediaResInfo1.FileTitle.strFileTitle, MediaResInfo2.FileTitle.setFileTitle);
	}
	else
	{
		return compareTitle(MediaResInfo1.FileTitle.strFileTitle, MediaResInfo2.FileTitle.strFileTitle);
	}
}

UINT CSimilarCheck::compareTitle(const SSet<wstring>& setFileTitle1, const SSet<wstring>& setFileTitle2)
{
	size_t uMatchSize = 0;
	UINT uMatchCount = 0;

	SVector<wstring> vecFileTitle2(setFileTitle2);
	for (auto& strTitle : setFileTitle1)
	{
		for (auto itr = vecFileTitle2.begin(); itr != vecFileTitle2.end(); ++itr)
		{
			if (compareTitle(strTitle, *itr) > 0)
			{
				uMatchSize += min(strTitle.size(), itr->size());

				uMatchCount++;

				itr = vecFileTitle2.erase(itr);

				break;
			}
		}
	}

	if (uMatchCount >= 2)
	{
		UINT uTotalSize1 = 0;
		for (auto& strTitle : setFileTitle1)
		{
			uTotalSize1 += strTitle.size();
		}

		UINT uTotalSize2 = 0;
		for (auto& strTitle : setFileTitle2)
		{
			uTotalSize2 += strTitle.size();
		}

		return uMatchSize / max(uTotalSize1, uTotalSize2) * 100;
	}
	
	return 0;
}

UINT CSimilarCheck::compareTitle(const wstring& strFileTitle1, const SSet<wstring>& setFileTitle2)
{
	size_t uMatchSize = 0;
	UINT uMatchCount = 0;
	for (auto& strTitle : setFileTitle2)
	{
		if (0 != compareTitleEx(strFileTitle1, strTitle))
		{
			uMatchSize += strTitle.size();
		
			uMatchCount++;
		}
	}

	if (uMatchCount >=2)
	{
		return uMatchSize / strFileTitle1.size() * 100;
	}

	return 0;
}

UINT CSimilarCheck::compareTitle(const wstring& strFileTitle1, const wstring& strFileTitle2)
{
	if (strFileTitle1.size() != strFileTitle2.size())
	{
		if (strFileTitle1.size() <= 2)
		{
			if (!util::checkWChar(strFileTitle1))
			{
				return 0;
			}
		}

		if (strFileTitle2.size() <= 2)
		{
			if (!util::checkWChar(strFileTitle2))
			{
				return 0;
			}
		}
	}

	if (wstring::npos != strFileTitle1.find(strFileTitle2))
	{
		return UINT(100 * strFileTitle2.size() / strFileTitle1.size());
	}
	else if (wstring::npos != strFileTitle2.find(strFileTitle1))
	{
		return UINT(100 * strFileTitle1.size() / strFileTitle2.size());
	}

	return 0;
}

UINT CSimilarCheck::compareTitleEx(const wstring& strFileTitle1, const wstring& strFileTitle2)
{
	size_t uSize2 = strFileTitle2.size();
	if (strFileTitle1.size() > uSize2)
	{
		if (uSize2 <= 2)
		{
			if (!util::checkWChar(strFileTitle2))
			{
				return 0;
			}
		}

		if (wstring::npos != strFileTitle1.find(strFileTitle2))
		{
			return UINT(100 * strFileTitle2.size() / strFileTitle1.size());
		}
	}
	else if (strFileTitle1.size() == uSize2)
	{
		if (strFileTitle1 == strFileTitle2)
		{
			return 100;
		}
	}

	return 0;
}
