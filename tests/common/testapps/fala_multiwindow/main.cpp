/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applifed.
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

#include "multiwindowcontent.h"
#include <QString>
#include <MApplication>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif


M_EXPORT int main(int argc, char **argv)
{
#ifdef HAVE_MCOMPONENTCACHE
    MApplication *app = MComponentCache::mApplication(argc, argv);

    bool bWindowNotFromCache = false;
    const QString sWindowNotFromCache = "window-not-from-cache";

    for (int i = 1; i < argc; i++) {
        QString sArg = QString(argv[i]);
        if (sArg.contains(sWindowNotFromCache,Qt::CaseInsensitive)) {
            bWindowNotFromCache = true;
            break;
        }
    }

    if (bWindowNotFromCache) {
        MultiWindowContent mwContent(false);
        mwContent.createWindows();
        mwContent.activateWindow(1);
    } else {
        MultiWindowContent mwContent(true);
        mwContent.createWindows();
        mwContent.activateWindow(1);
    }

#else
    MApplication *app = new MApplication(argc, argv);
    MultiWindowContent mwContent(false);
    mwContent.createWindows();
    mwContent.activateWindow(1);
#endif

    return app->exec();
}
