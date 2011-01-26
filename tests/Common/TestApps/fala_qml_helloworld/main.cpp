/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include <Qt/QtDeclarative>
#include <QFile>
#include <sys/time.h>
#include "qdeclarativeboostercache.h"

void FANGORNLOG(const char* s)
{
    QFile f("/tmp/testapp.log");
    f.open(QIODevice::Append);
    f.write(s, qstrlen(s));
    f.write("\n", 1);
    f.close();
}

void timestamp(const char *s)
{
    timeval tim;
    char msg[80];
    gettimeofday(&tim, NULL);
    snprintf(msg, 80, "%d%03d %s\n", 
             static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec/1000), s);
    FANGORNLOG(msg);
}

Q_DECL_EXPORT int main(int argc, char **argv)
{
    timestamp("application main");

    QApplication *app = QDeclarativeBoosterCache::qApplication(argc, argv);
    timestamp("QApplication from cache");

    QDeclarativeView *window = QDeclarativeBoosterCache::qDeclarativeView();
    timestamp("QDeclarativeView from cache");
    
    window->setWindowTitle("Applauncherd QML testapp");

    window->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    window->setSource(QUrl::fromLocalFile("/usr/share/fala_qml_helloworld/main.qml"));
    window->showFullScreen();
    
    timestamp("Calling app->exec()");
    return app->exec();
}

