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

#ifndef MDECLARATIVECACHE_P_H
#define MDECLARATIVECACHE_P_H

class MDeclarativeCache;
class QApplication;
class QDeclarativeView;

class MDeclarativeCachePrivate
{
public:
    MDeclarativeCachePrivate();
    virtual ~MDeclarativeCachePrivate();
    void populate();
    QApplication* qApplication(int &argc, char **argv);
    QDeclarativeView* qDeclarativeView();

    static const int ARGV_LIMIT;
    QApplication *qApplicationInstance;
    QDeclarativeView *qDeclarativeViewInstance;
    int initialArgc;
    char **initialArgv;

#ifdef UNIT_TEST
    friend class Ut_MDeclarativeCache;
#endif
};

#endif // MDECLARATIVECACHE_P_H
