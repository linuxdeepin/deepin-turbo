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
#include <QtPlugin>
#include <QPluginLoader>
#include <QLibraryInfo>

#include "mdeclarativecache.h"
#include "mdeclarativecache_p.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include "coverage.h"

MDeclarativeCachePrivate * const MDeclarativeCache::d_ptr = new MDeclarativeCachePrivate;
const int MDeclarativeCachePrivate::ARGV_LIMIT = 32;

MDeclarativeCachePrivate::MDeclarativeCachePrivate() :
    qApplicationInstance(0),
    qDeclarativeViewInstance(0),
    initialArgc(ARGV_LIMIT),
    initialArgv(new char* [initialArgc]),
    appDirPath(QString()),
    appFilePath(QString()),
    cachePopulated(false)
{
}

MDeclarativeCachePrivate::~MDeclarativeCachePrivate()
{
    delete qDeclarativeViewInstance;
    delete[] initialArgv;

#ifdef WITH_COVERAGE
    __gcov_flush();
#endif
}

void MDeclarativeCachePrivate::populate()
{
    // Record the fact that the cache has been populated
    cachePopulated = true;
    
    static const char *const emptyString = "";
    static const QString appNameFormat = "mdeclarativecache_pre_initialized_qapplication-%1";
    static QByteArray appName;

    // Append pid to appName to make it unique. This is required because the
    // libminputcontext.so instantiates MComponentData, which in turn registers
    // a dbus service with the application's name.
    appName = appNameFormat.arg(getpid()).toLatin1();

    // We support at most ARGV_LIMIT arguments in QCoreApplication. These will be set when real
    // arguments are known (in MDeclarativeCachePrivate::qApplication). 
    initialArgv[0] = const_cast<char *>(appName.constData());
    for (int i = 1; i < initialArgc; i++) {
        initialArgv[i] = const_cast<char *>(emptyString);
    }

    if (qApplicationInstance == 0) {
        qApplicationInstance = new QApplication(initialArgc, initialArgv);
    }

    qDeclarativeViewInstance = new QDeclarativeView();

}

QApplication* MDeclarativeCachePrivate::qApplication(int &argc, char **argv)
{
    if (qApplicationInstance == 0) 
    {
        qApplicationInstance = new QApplication(argc, argv);
    } 
    else 
    {
        if (argc > ARGV_LIMIT) 
        {
            qWarning("MDeclarativeCache: QCoreApplication::arguments() will not contain all arguments.");
        }
        
        // Copy arguments to QCoreApplication 
        for (int i = 0; i < qMin(argc, ARGV_LIMIT); i++) 
        {
            qApp->argv()[i] = argv[i];
        }
        
        // This changes argc in QCoreApplication
        initialArgc = qMin(argc, ARGV_LIMIT);

        // Take application name from argv
        QString appName = QFileInfo(argv[0]).fileName();

        // Set object name
        qApp->setObjectName(appName);

        bool loadTestabilityArg = false;
        const char* testabilityArg = "-testability";
        for (int i = 0; i < argc; i++) 
        {
            if (strcmp(argv[i], testabilityArg) == 0)
            {
                loadTestabilityArg = true;
                break;
            }
        }
        
        bool loadTestabilityEnv = !qgetenv("QT_LOAD_TESTABILITY").isNull();
        if (loadTestabilityEnv || loadTestabilityArg)
            testabilityInit();

#ifdef Q_WS_X11
        // reinit WM_COMMAND X11 property
        if (qDeclarativeViewInstance) 
        {
            Display *display = QX11Info::display();
            if (display) 
            {

                qDeclarativeViewInstance->winId();
                XSetCommand(display, qDeclarativeViewInstance->effectiveWinId(), argv, argc);

                // set correct WM_CLASS properties
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
        if (cachePopulated) 
        {
            // In Qt 4.7, QCoreApplication::applicationDirPath() and
            // QCoreApplication::applicationFilePath() look up the paths in /proc,
            // which does not work when the booster is used. As a workaround, we
            // use argv[0] to provide the correct values in the cache class.
            appFilePath = QString(argv[0]);
            appDirPath = QString(argv[0]);
            appDirPath.chop(appDirPath.size() - appDirPath.lastIndexOf("/"));
        }

    }

#ifdef WITH_COVERAGE
    __gcov_flush();
#endif

#ifdef HAVE_PATH_REINIT
    // Set the magic attribute so that paths are reinitialized
    qApplicationInstance->setAttribute(Qt::AA_LinuxReinitPathsFromArgv0, true);
#endif

    return qApplicationInstance;
}

void MDeclarativeCachePrivate::testabilityInit()
{
    // Activate testability plugin if exists
    QString testabilityPluginPostfix = ".so";
    QString testabilityPlugin = "testability/libtestability";

    testabilityPlugin = QLibraryInfo::location(QLibraryInfo::PluginsPath) + QDir::separator() + testabilityPlugin + testabilityPluginPostfix;
    QPluginLoader loader(testabilityPlugin.toLatin1().data());

    QObject *plugin = loader.instance();
    
    if (plugin) 
    {
        testabilityInterface = qobject_cast<TestabilityInterface *>(plugin);

        if (testabilityInterface) 
        {
            testabilityInterface->Initialize();
        }
    }
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

#ifdef WITH_COVERAGE
    __gcov_flush();
#endif

    return returnValue;
}

QString MDeclarativeCachePrivate::applicationDirPath()
{
    if (cachePopulated) {
        // In the booster case use the workaround
        return appDirPath;
    } else {
        return QCoreApplication::applicationDirPath();
    }
}

QString MDeclarativeCachePrivate::applicationFilePath()
{
    if (cachePopulated) {
        // In the booster case use the workaround
        return appFilePath;
    } else {
        return QCoreApplication::applicationFilePath();
    }
}

void MDeclarativeCache::populate()
{
    d_ptr->populate();

#ifdef WITH_COVERAGE
    __gcov_flush();
#endif
}

QApplication* MDeclarativeCache::qApplication(int &argc, char **argv)
{    
    return d_ptr->qApplication(argc, argv);
}

QDeclarativeView* MDeclarativeCache::qDeclarativeView()
{
    return d_ptr->qDeclarativeView();
}

QString MDeclarativeCache::applicationDirPath()
{
    return d_ptr->applicationDirPath();
}

QString MDeclarativeCache::applicationFilePath()
{
    return d_ptr->applicationFilePath();
}
