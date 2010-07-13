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

void FANGORNLOG(const char* s)
{
    QFile f("/tmp/testapp.log");
    f.open(QIODevice::Append);
    f.write(s, qstrlen(s));
    f.write("\n", 1);
    f.close();
}

void timestamp(const char *s)
{
    timeval tim;
    char msg[80];
    gettimeofday(&tim, NULL);
    snprintf(msg, 80, "%d.%06d %s\n", 
             static_cast<int>(tim.tv_sec), static_cast<int>(tim.tv_usec), s);
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
    timestamp("application main");
    
#ifdef HAVE_MCOMPONENTCACHE   
    MApplication* app = MComponentCache::mApplication(argc, argv);
    MApplicationWindow* w = MComponentCache::mApplicationWindow();
#else
    MApplication* app = new MApplication(argc, argv);
    MApplicationWindow* w = new MApplicationWindow;
#endif
    
    MyApplicationPage p;
    MApplication::setPrestartMode(M::LazyShutdown);
    p.setTitle("Applauncherd testapp");
    w->show();
    p.appear();
    return app->exec();
}

