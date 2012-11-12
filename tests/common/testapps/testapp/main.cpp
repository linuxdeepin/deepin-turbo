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

#include <QApplication>
#include <QFile>
#include <QBrush>
#include <QColor>
#include <sys/time.h>

#include <exception>

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

Q_DECL_EXPORT int main(int argc, char **argv)
{
    QApplication *app;
    try
    {
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

        app = new QApplication(argc, argv);
        timestamp("app created without cache");

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
    }
    catch(std::exception& e)
    {
        return -1;
    }

    _exit(app->exec());
}

