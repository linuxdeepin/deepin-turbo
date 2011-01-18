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
#include "qdeclarativeboostercache.h"
#include "qdeclarativeboostercache_p.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

QDeclarativeBoosterCachePrivate * const QDeclarativeBoosterCache::d_ptr = new QDeclarativeBoosterCachePrivate;
const int QDeclarativeBoosterCachePrivate::ARGV_LIMIT = 32;

QDeclarativeBoosterCachePrivate::QDeclarativeBoosterCachePrivate() :
    qApplicationInstance(0),
    qDeclarativeViewInstance(0),
    initialArgc(ARGV_LIMIT),
    initialArgv(new char* [initialArgc])
{
}

QDeclarativeBoosterCachePrivate::~QDeclarativeBoosterCachePrivate()
{
    delete qDeclarativeViewInstance;
    delete[] initialArgv;
}

void QDeclarativeBoosterCachePrivate::populate()
{
    
    static const char *const emptyString = "";
    static const char *const appNameFormat = "qdeclarativeboostercache_pre_initialized_qapplication%d";
    static char appName[strlen(appNameFormat) + 8];

    // Append pid to appName to make it unique. This is required because the
    // libminputcontext.so instantiates MComponentData, which in turn registers
    // a dbus service with the application's name.
    sprintf(appName, appNameFormat, getpid());

    // We support at most ARGV_LIMIT arguments in QCoreApplication. These will be set when real
    // arguments are known (in QDeclarativeBoosterCachePrivate::qApplication). 
    initialArgv[0] = const_cast<char *>(appName);
    for (int i = 1; i < initialArgc; i++) {
        initialArgv[i] = const_cast<char *>(emptyString);
    }

    if (qApplicationInstance == 0) {
        qApplicationInstance = new QApplication(initialArgc, initialArgv);
    }

    qDeclarativeViewInstance = new QDeclarativeView();
}

QApplication* QDeclarativeBoosterCachePrivate::qApplication(int &argc, char **argv)
{
    if (qApplicationInstance == 0) {
        qApplicationInstance = new QApplication(argc, argv);
    } else {
        if (canUseCachedApp(argc, argv)) {
            if(argc > ARGV_LIMIT) {
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
                }
            }
#endif
        } else {
	    // Clean up cache.
            if (qDeclarativeViewInstance) {
                delete qDeclarativeViewInstance;
                qDeclarativeViewInstance = 0;
            }

            delete qApplicationInstance;
            qApplicationInstance = new QApplication(argc, argv);
        }
    }
    return qApplicationInstance;
}

bool QDeclarativeBoosterCachePrivate::canUseCachedApp(int &argc, char **argv)
{
    if ( hasExtraParams(argc, argv) )
        return false;

    return true;
}

bool QDeclarativeBoosterCachePrivate::hasExtraParams(int &argc, char **argv)
{
    for (int i = 1; i < argc; ++i) 
    {
        QString s(argv[i]);

        if (
            /* QApplication command options */
            (s == "-style")                       ||
            (s == "-stylesheet")                  ||
            (s == "-session")                     ||
            (s == "-widgetcount")                 ||
            (s == "-reverse")                     ||
            (s == "-graphicssystem")              ||

            /* X11 options */
            (s == "-display")                     ||
            (s == "-geometry" )                   ||
            (s == "-fn" )                         ||
            (s == "-font")                        ||
            (s == "-bg")                          ||
            (s == "-background")                  ||
            (s == "-fg")                          ||
            (s == "-foreground")                  ||
            (s == "-btn")                         ||
            (s == "-button")                      ||
            (s == "-name" )                       ||
            (s == "-title" )                      ||
            (s == "-visual" )                     ||
            (s == "-ncols")                       ||
            (s == "-cmap")                        ||
            (s == "-im")                          ||
            (s == "-inputstyle" )                 ||

            /* help options, application will not be started */
            (s == "-v")                           ||
            (s.startsWith("-version"))            ||
            (s.startsWith("--version"))           ||
            (s == "-h")                           ||
            (s.startsWith("-help"))               ||
            (s.startsWith("--help"))
           )
            return true;
    }
    return false;
}

QDeclarativeView* QDeclarativeBoosterCachePrivate::qDeclarativeView()
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

void QDeclarativeBoosterCache::populate()
{
    d_ptr->populate();
}

QApplication* QDeclarativeBoosterCache::qApplication(int &argc, char **argv)
{    
    return d_ptr->qApplication(argc, argv);
}

QDeclarativeView* QDeclarativeBoosterCache::qDeclarativeView()
{
    return d_ptr->qDeclarativeView();
}
