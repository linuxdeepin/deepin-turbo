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
#include <MExport>
#include <QTimer>
#include <iostream>

#ifdef HAVE_MCOMPONENTCACHE
#include <mcomponentcache.h>
#endif

M_EXPORT int main(int argc, char ** argv)
{
#ifdef HAVE_MCOMPONENTCACHE
    MApplication *app = MComponentCache::mApplication(argc, argv);
#endif
    QTimer::singleShot(5, app, SLOT(quit()));
    int usr_id = getuid();
    int grp_id = getgid();

    std::cerr << "uid=" << usr_id <<"\n";
    std::cerr << "gid=" << grp_id <<"\n";
    app->exec();
    return 29;
    
}
