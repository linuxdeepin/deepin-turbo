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
#include <MApplicationWindow>
#include <MApplicationPage>

#ifdef HAVE_MCOMPONENTCACHE
    #include <MComponentCache>
#endif

#include <MExport>
#include <QFile>
#include <sys/time.h>

QString log_file = "/tmp/fala_testapp.log";

void FANGORNLOG(const char* s, bool eol = true)
{
    QFile f(log_file);
    f.open(QIODevice::Append);
    f.write(s, qstrlen(s));
    if (eol) {
        f.write("\n", 1);
    }
    f.close();
}

void FANGORNLOG(const QString& s, bool eol = true)
{
    QByteArray ba = s.toLocal8Bit();
    char *p = new char[ba.size() + 1];
    strcpy(p, ba.data());
    FANGORNLOG(p, eol);
}

void timestamp(const char *s)
{
    timeval tim;
    char msg[80];
    gettimeofday(&tim, NULL);
    snprintf(msg, 80, "%d%03d %s", 
             static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec)/1000, s);
    FANGORNLOG(msg);
}

class MyApplicationPage: public MApplicationPage
{
public:
    MyApplicationPage(): MApplicationPage() {}
    virtual ~MyApplicationPage() {}
    void enterDisplayEvent() {
        timestamp("MyApplicationPage::enterDisplayEvent");
    }
};

M_EXPORT int main(int, char**);

int main(int argc, char **argv) {
    QString appName(argv[0]);
    if (appName.endsWith("fala_wl"))
    {
        log_file = "/tmp/fala_wl.log";
    }
    else if (appName.endsWith("fala_wol"))
    {
        log_file = "/tmp/fala_wol.log";
    }
    timestamp("application main");
#ifdef HAVE_MCOMPONENTCACHE
    MApplication* app = MComponentCache::mApplication(argc, argv);
    timestamp("app from cache");
    MApplicationWindow* w = MComponentCache::mApplicationWindow();
    timestamp("win from cache");

#else
    MApplication* app = new MApplication(argc, argv);
    timestamp("app created without cache");

    MApplicationWindow* w = new MApplicationWindow;
    timestamp("win created without cache");
#endif

    if (argc > 2 && QString(argv[1]) == QString("--log-args")) {
        FANGORNLOG("argv:", false);
        for (int i = 0; i < argc; i++) {
            FANGORNLOG(" ", false);
            FANGORNLOG(argv[i], false);
        }
        FANGORNLOG("");

        FANGORNLOG("argv:", false);
        QStringList args = QCoreApplication::arguments();
        for (int i = 0; i < args.size(); i++) {
            FANGORNLOG(" ", false);
            FANGORNLOG(args.at(i), false);
        }
        FANGORNLOG("");
    }

    MyApplicationPage p;
    timestamp("page created");

    MApplication::setPrestartMode(M::LazyShutdown);
    p.setTitle("Applauncherd testapp");

    p.appear();
    timestamp("page.appear() called");

    w->show();
    timestamp("w->show() called");

    _exit(app->exec());
}

