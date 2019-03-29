
#pragma once

#include <util/util.h>

#define __Space L' '

#define __CNDotDelimiter L" · "

#define __TimeFormat wstring(L"%y-%m-%d\n %H:%M")

#ifdef __MediaLibPrj
#define __MediaLibExt __declspec(dllexport)
#else
#define __MediaLibExt __declspec(dllimport)
#endif

class IMedia;
using TD_IMediaList = PtrArray<IMedia>;
using TD_IMediaMap = map<wstring, TD_IMediaList>;

class CMedia;
using TD_MediaList = PtrArray<CMedia>;
using TD_MediaMap = map<wstring, TD_MediaList>;

class CMediaSet;
using TD_MediaSetList = PtrArray<CMediaSet>;

class CMediaRes;
using TD_MediaResList = PtrArray<CMediaRes>;

class CPlayItem;
using TD_PlayItemList = PtrArray<CPlayItem>;

class CAlbumItem;
using TD_AlbumItemList = PtrArray<CAlbumItem>;

enum class E_GlobalImage
{
	GI_Folder = 0
	, GI_FolderLink
	, GI_MediaFile
	, GI_DiskFile

	, GI_Playlist
	, GI_PlayItem

	, GI_SingerGroup
	, GI_SingerDefault
	, GI_Album
	, GI_AlbumItem
};

enum class E_MediaSetType
{
	MST_Null = -1
	, MST_Playlist
	, MST_Album
	, MST_Singer
	, MST_SingerGroup
};

class __MediaLibExt CMediaTime
{
public:
	CMediaTime()
		: m_time(time(0))
	{
	}

	CMediaTime(time_t time)
		: m_time(time)
	{
	}

public:
	time_t m_time;

public:
	boolean operator <(CMediaTime time) const
	{
		return m_time < time.m_time;
	}

	wstring GetText() const
	{
		return util::FormatTime(m_time, __TimeFormat);
	}
};

enum class E_MediaSetChanged
{
	MSC_Rename
	, MSC_Remove

	, MSC_SingerImgChanged
};

struct tagMediaSetChanged
{
	tagMediaSetChanged(E_MediaSetType t_eMediaSetType, E_MediaSetChanged t_eChangedType, UINT t_uMediaSetID)
		: eMediaSetType(t_eMediaSetType)
		, eChangedType(t_eChangedType)
		, uMediaSetID(t_uMediaSetID)
	{
	}

	E_MediaSetType eMediaSetType;

	E_MediaSetChanged eChangedType;

	UINT uMediaSetID;

	wstring strNewName;

	UINT uSingerImgPos = 0;
};

struct tagRelatedMediaSet
{
	int iMediaSetID = -1;
	wstring strMediaSetName;

	int iMediaID = -1;

	void clear()
	{
		iMediaSetID = -1;
		strMediaSetName.clear();

		iMediaID = -1;
	}
};

enum class E_FindMediaMode
{
	FMM_MatchText = 0

	, FMM_MatchPath
	
	, FMM_MatchDir

	, FMM_MatchFiles

	, FMM_RenameDir
};

struct tagFindMediaPara
{
	tagFindMediaPara()
	{
	}

	tagFindMediaPara(E_FindMediaMode t_eFindMediaMode)
	{
		eFindMediaMode = t_eFindMediaMode;
	}

	tagFindMediaPara(E_FindMediaMode t_eFindMediaMode, const wstring& t_strFindText
		, E_MediaSetType t_eFindMediaSetType = E_MediaSetType::MST_Null, bool t_bFindOne = false)
	{
		eFindMediaMode = t_eFindMediaMode;
		strFindText = t_strFindText;

		eFindMediaSetType = t_eFindMediaSetType;
		bFindOne = t_bFindOne;
	}

	E_FindMediaMode eFindMediaMode = E_FindMediaMode::FMM_MatchText;
	wstring strFindText;

	E_MediaSetType eFindMediaSetType = E_MediaSetType::MST_Null;

	bool bFindOne = false;

	wstring strFindSingerName;

	wstring strDir;

	wstring strRenameDir;

	std::set<wstring> setFiles;
};

enum class E_RenameRetCode
{
	RRC_Success = 0

	, RRC_Failure

	, RRC_NameExists
};

#include "IMedia.h"

#include "MediaRes.h"

#include "RootMediaRes.h"

#include "Media.h"

#include "MediaSet.h"

#include <PlaylistMedia.h>

#include <SingerMedia.h>

#include <MediaMixture.h>
