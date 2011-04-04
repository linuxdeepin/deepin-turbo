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
#include <MDeclarativeCache>

Q_DECL_EXPORT int main(int argc, char **argv)
{
    QApplication *app = MDeclarativeCache::qApplication(argc, argv);
    QDeclarativeView *window = MDeclarativeCache::qDeclarativeView();

    // Use MDeclarativeCache::applicationDirPath()
    // instead of QCoreApplication::applicationDirPath()
    // because the latter does not work with the booster.
    QDir::setCurrent(MDeclarativeCache::applicationDirPath());

    window->setSource(QUrl::fromLocalFile("main.qml"));

    window->showFullScreen();
    return app->exec();
}

