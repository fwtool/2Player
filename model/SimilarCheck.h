#pragma once

using TD_SimilarGroupMap = SMap<UINT, TD_SimilarFileGroup>;

class CSimilarCheck
{
public:
	static void checkSimilarFile(PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo
		, CB_checkSimilarFile cb, TD_SimilarFile& arrResult);

	static void checkSimilarFile(PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo1
		, PairList<CMediaRes*, tagMediaResInfo>& plMediaResInfo2
		, CB_checkSimilarFile cb, TD_SimilarFile& arrResult);

private:
	static UINT compareMediaResInfo(const tagMediaResInfo& MediaResInfo1, const tagMediaResInfo& MediaResInfo2);

	static UINT compareTitle(const SSet<wstring>& setFileTitle1, const SSet<wstring>& setFileTitle2);
	
	static UINT compareTitle(const wstring& strFileTitle1, const SSet<wstring>& setFileTitle2);

	static UINT compareTitle(const wstring& strFileTitle1, const wstring& strFileTitle2);

	static UINT compareTitleEx(const wstring& strFileTitle1, const wstring& strFileTitle2);
};
