/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <MApplication>
#include <MApplicationPage>
#include <MApplicationWindow>
#include <MLayout>
#include <QTimer>
#include <MLinearLayoutPolicy>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

M_EXPORT int main(int argc, char ** argv)
{
#ifdef HAVE_MCOMPONENTCACHE
    MApplication *app = MComponentCache::mApplication(argc, argv);
    MApplicationWindow *window = MComponentCache::mApplicationWindow();
#else
    MApplication *app = new MApplication(argc, argv);
    MApplicationWindow *window = new MApplicationWindow;
#endif

    MApplicationPage *mainPage = new MApplicationPage;
    window->show();
    mainPage->appear(window);
    QTimer::singleShot(3000, app, SLOT(quit()));
    exit(app->exec());
}
