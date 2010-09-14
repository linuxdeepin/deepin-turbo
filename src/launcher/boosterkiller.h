#ifndef BOOSTERKILLER_H
#define BOOSTERKILLER_H

#include <QObject>
#include <QStringList>

class QString;
class MGConfItem;

class BoosterKiller: public QObject
{
    Q_OBJECT

 public:
    /* Add a GConf key and the name of the process that should be
       killed in case the value associated to the key is changed.
    */
    void addKey(const QString & key);
    void addProcessName(const QString & processName);

    /* Starts the killer. This will initialize qcoreapplication, does
       not return.
     */
    void start();

 private slots:
    void killProcesses();
    
 private:
    QList<MGConfItem*> gconfItems;
    QStringList processNames;
    
};

#endif
