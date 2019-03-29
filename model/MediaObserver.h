
#pragma once

class CMediaObserver : public IMediaObserver
{
private:
	CMediaObserver(class CModel& model, IModelObserver& ModelObserver)
		: m_model(model)
		, m_ModelObserver(ModelObserver)
	{
	}

public:
	static IMediaObserver& createInst(class CModel& model, IModelObserver& ModelObserver)
	{
		static CMediaObserver inst(model, ModelObserver);
		return inst;
	}

private:
	CModel& m_model;

	IModelObserver& m_ModelObserver;

private:
	void renameMedia(IMedia& media, const wstring& strNewName) override;

	CMedia *findRelatedMedia(IMedia& media, E_MediaSetType eMediaSetType) override;

	int checkMedia(const wstring& strAbsPath) override;

	UINT getSingerImgPos(UINT uSingerID) override;
};
