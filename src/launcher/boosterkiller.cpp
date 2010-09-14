#include <QString>
#include <QCoreApplication>
#include <QObject>
#include <MGConfItem>
#include "boosterkiller.h"

#include <cstdlib>

void BoosterKiller::addKey(const QString & key)
{
    MGConfItem *item = new MGConfItem(key, 0);
    QObject::connect(item, SIGNAL(valueChanged()),
                     this, SLOT(killProcesses()));
}

void BoosterKiller::addProcessName(const QString & processName)
{
    processNames << processName;
}

void BoosterKiller::start()
{
    int argc = 0;
    //char **argv = 0;
    QCoreApplication(argc, 0).exec();
}

void BoosterKiller::killProcesses()
{
    Q_FOREACH(QString processName, processNames) {
        system( (QString("pkill ") + processName).toStdString().c_str() );
    }
}
