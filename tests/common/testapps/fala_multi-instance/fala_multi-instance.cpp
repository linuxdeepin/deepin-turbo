#include <MApplication>
#include <MApplicationWindow>
#include <MApplicationService>
#include <MApplicationPage>
#include <MComponentCache>
#include <MExport>
#include "mainpage.h"
#include "mainwindow.h"

M_EXPORT int main(int argc, char ** argv)
{
    QString servicename(QString("com.nokia.multihello") + QString(argv[1]));
    QString title(QString("App ")+ QString(argv[1])); 
    MApplicationService *mService = new MApplicationService(servicename, 0);
    MApplication * app = MComponentCache::mApplication(argc, argv, "multihello", mService);
    MainWindow window;
    MainPage mainPage;
    mainPage.setName(title);

    window.show();

    mainPage.appear();
  
    return app->exec();
}

