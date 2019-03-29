#pragma once

#include <Common.h>

#include <model.h>

class IPlayerView
{
public:
	virtual IView& getView() = 0;

	virtual IModelObserver& getModelObserver() = 0;
	
	virtual bool showMsgBox(const wstring& strMsg, bool bFlag = false) = 0;

	virtual wstring showFolderDlg(const wstring& strTitle, const wstring& strTipMsg, HWND hWnd = NULL) = 0;
	
	virtual void updateMeidaRelated(tagMediaSetChanged MediaSetChanged) = 0;

	virtual void updateMediaResPanel(CMediaRes& MediaRes) = 0;
	
	virtual void onAlarm(const wstring& strAlarmmedia) = 0;

	virtual void onTimerOperate(E_TimerOperate eTimerOperate) = 0;
};

using CB_AddInConfirm = function<E_MatchResult(CSearchMediaInfo&, tagMediaResInfo&)>;

interface IPlayerController
{
	virtual bool addPlayingItem(const TD_IMediaList& lstMedias, int iPos = -1) = 0;
	virtual bool addPlayingItem(CMediaSet& MediaSet) = 0;
		
	virtual bool renameMedia(IMedia& media, const wstring& strNewName) = 0;

	virtual void moveMediaRes(const TD_IMediaList& lstMedias, const wstring& strDir) = 0;

	virtual bool removeMediaRes(const TD_MediaResList& lstMediaRes, TD_MediaResList& lstDeletedMediaRes) = 0;

	virtual int addPlayItems(const list<wstring>& lstFiles, CPlaylist& Playlist) = 0;

	virtual int addAlbumItems(const list<wstring>& lstFiles, CAlbum& Album) = 0;

	virtual E_RenameRetCode renameMediaSet(CMediaSet& MediaSet, const wstring& strNewName) = 0;

	virtual bool removeMediaSet(CMediaSet& MediaSet) = 0;

	virtual bool autoMatchMedia(CMediaRes& SrcPath, const TD_MediaList& lstMedias, const CB_AutoMatchProgress& cbProgress
		, const CB_AutoMatchConfirm& cbConfirm, map<CMedia*, wstring>& mapUpdatedMedias) = 0;

	using CB_AddInMediaProgress = function<bool(const wstring& strFile)>;
	virtual UINT addInMedia(const list<wstring>& lstFiles, const CB_AddInMediaProgress& cbProgress, const CB_AddInConfirm& cbAddInConfirm) = 0;
};

#ifdef __ViewPrj
#define __ViewExt __declspec(dllexport)
#else
#define __ViewExt __declspec(dllimport)
#endif

__ViewExt IPlayerView& createView(IPlayerController& controller, IModel& model);
