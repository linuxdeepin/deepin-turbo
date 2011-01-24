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

#ifndef QDECLARATIVEBOOSTERCACHE_P_H
#define QDECLARATIVEBOOSTERCACHE_P_H

class QDeclarativeBoosterCache;
class QApplication;
class QDeclarativeView;

class QDeclarativeBoosterCachePrivate
{
public:
    QDeclarativeBoosterCachePrivate();
    virtual ~QDeclarativeBoosterCachePrivate();
    void populate();
    QApplication* qApplication(int &argc, char **argv);
    QDeclarativeView* qDeclarativeView();

    bool canUseCachedApp(int &argc, char **argv);
    bool hasExtraParams(int &argc, char **argv);

    static const int ARGV_LIMIT;
    QApplication *qApplicationInstance;
    QDeclarativeView *qDeclarativeViewInstance;
    int initialArgc;
    char **initialArgv;

#ifdef UNIT_TEST
    friend class Ut_QDeclarativeBoosterCache;
#endif
};

#endif // QDECLARATIVEBOOSTERCACHE_P_H
