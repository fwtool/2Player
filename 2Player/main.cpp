#include "mainwindow.h"
#include <QApplication>

#include "model.h"

class CModelObserver : public IModelObserver
{
    void refreshMediaRes() override {}

    void refreshPlayingList() override {}

    void onPlay(UINT uPlayingItem, UINT uPrevPlayingItem) override {}

    void onPlayFinish() override {}

    UINT GetSingerImgPos(UINT uSingerID) override {return 0;}

    void renameMedia(IMedia& media, const wstring& strNewName) override {}

    void updateMedia(const tagUpdateMedia& UpdateMedia) override {}
};

static CModelObserver g_ModelObserver;

static CModel g_model(g_ModelObserver);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!g_model.init())
    {
        return -1;
    }

    if (!g_model.getPlayMgr().getPlayingItems().empty())
    {
        g_model.getPlayMgr().play();
    }
    else
    {
        g_model.getPlayMgr().demandMediaSet(g_model.getMediaLib(), E_MediaSetType::MST_Album);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
