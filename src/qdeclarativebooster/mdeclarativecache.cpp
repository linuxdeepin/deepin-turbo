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
#include <QX11Info>
#include "mdeclarativecache.h"
#include "mdeclarativecache_p.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

MDeclarativeCachePrivate * const MDeclarativeCache::d_ptr = new MDeclarativeCachePrivate;
const int MDeclarativeCachePrivate::ARGV_LIMIT = 32;

MDeclarativeCachePrivate::MDeclarativeCachePrivate() :
    qApplicationInstance(0),
    qDeclarativeViewInstance(0),
    initialArgc(ARGV_LIMIT),
    initialArgv(new char* [initialArgc])
{
}

MDeclarativeCachePrivate::~MDeclarativeCachePrivate()
{
    delete qDeclarativeViewInstance;
    delete[] initialArgv;
}

void MDeclarativeCachePrivate::populate()
{
    
    static const char *const emptyString = "";
    static const char *const appNameFormat = "mdeclarativecache_pre_initialized_qapplication%d";
    static char appName[strlen(appNameFormat) + 8];

    // Append pid to appName to make it unique. This is required because the
    // libminputcontext.so instantiates MComponentData, which in turn registers
    // a dbus service with the application's name.
    sprintf(appName, appNameFormat, getpid());

    // We support at most ARGV_LIMIT arguments in QCoreApplication. These will be set when real
    // arguments are known (in MDeclarativeCachePrivate::qApplication). 
    initialArgv[0] = const_cast<char *>(appName);
    for (int i = 1; i < initialArgc; i++) {
        initialArgv[i] = const_cast<char *>(emptyString);
    }

    if (qApplicationInstance == 0) {
#ifdef __arm__
        QApplication::setGraphicsSystem("meego");
#else
        QApplication::setGraphicsSystem("raster");
#endif
        qApplicationInstance = new QApplication(initialArgc, initialArgv);
    }

    qDeclarativeViewInstance = new QDeclarativeView();
}

QApplication* MDeclarativeCachePrivate::qApplication(int &argc, char **argv)
{
    if (qApplicationInstance == 0) {
        qApplicationInstance = new QApplication(argc, argv);
    } else {
        if (argc > ARGV_LIMIT) {
            qWarning("MComponentCache: QCoreApplication::arguments() will not contain all arguments.");
        }
        
        // Copy arguments to QCoreApplication 
        for (int i = 0; i < qMin(argc, ARGV_LIMIT); i++) {
            qApp->argv()[i] = argv[i];
        }
        
        // This changes argc in QCoreApplication
        initialArgc = qMin(argc, ARGV_LIMIT);
        
#ifdef Q_WS_X11
        // reinit WM_COMMAND X11 property
        if (qDeclarativeViewInstance) {
            Display *display = QX11Info::display();
            if (display) {
                XSetCommand(display, qDeclarativeViewInstance->effectiveWinId(), argv, argc);

                // set correct WM_CLASS properties
		QString appName = QFileInfo(argv[0]).fileName();
		QString appClass = appName.left(1).toUpper();
		if (appName.length() > 1)
                    appClass += appName.right(appName.length() - 1);
		
		// reserve memory for C strings
		QByteArray arrName(appName.toLatin1());
		QByteArray arrClass(appClass.toLatin1());
		
		XClassHint class_hint;
		class_hint.res_name = arrName.data();
		class_hint.res_class = arrClass.data();
		
		XSetClassHint(display, qDeclarativeViewInstance->effectiveWinId(), &class_hint);
            }
        }
#endif
    }
    return qApplicationInstance;
}

QDeclarativeView* MDeclarativeCachePrivate::qDeclarativeView()
{
    QDeclarativeView *returnValue;
    if (qDeclarativeViewInstance != 0) {
        returnValue = qDeclarativeViewInstance;
        qDeclarativeViewInstance = 0;
    } else {
        returnValue = new QDeclarativeView();
    }
    return returnValue;
}

void MDeclarativeCache::populate()
{
    d_ptr->populate();
}

QApplication* MDeclarativeCache::qApplication(int &argc, char **argv)
{    
    return d_ptr->qApplication(argc, argv);
}

QDeclarativeView* MDeclarativeCache::qDeclarativeView()
{
    return d_ptr->qDeclarativeView();
}

