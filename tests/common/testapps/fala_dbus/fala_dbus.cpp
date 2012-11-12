/***************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
#include <QDBusConnection>
#include <QDBusMessage>
#include <QString>
#include <syslog.h>
#include <iostream>

Q_DECL_EXPORT int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
    Q_UNUSED(app);
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.dsme", "/com/nokia/dsme/request", "com.nokia.dsme.request", "req_powerup");
    QDBusMessage reply = bus.call(msg,QDBus::AutoDetect, 5);
    QString dbusReplyError = reply.errorName();
    openlog ("qfala_dbus", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_DAEMON);
    syslog (LOG_DAEMON | LOG_NOTICE, "fala_dbus connection error: '%s'", dbusReplyError.toLocal8Bit().data());    
    closelog();
    std::cerr << "fala_dbus connection error:" << dbusReplyError.toLocal8Bit().data() << "\n";
    _exit(0);
}
