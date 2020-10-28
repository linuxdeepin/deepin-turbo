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

#include "booster-dtkwidget.h"
#include "daemon.h"

#include <DApplication>
#include <QWidget>
#include <QImageReader>

const string QWBooster::m_boosterType  = "dtkwidget";

const string & QWBooster::boosterType() const
{
    return m_boosterType;
}

void QWBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    static int argc = initialArgc;
    Q_UNUSED(new DTK_WIDGET_NAMESPACE::DApplication(argc, initialArgv));

    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

bool QWBooster::preload()
{
    // 初始化QWidget，减少程序启动后第一次显示QWidget时的时间占用
    // 在龙芯和申威上，时间主要消耗在xcb插件中加载glx相关库（libdri600等）
    QWidget widget;

    widget.setWindowFlags(Qt::BypassWindowManagerHint
                          | Qt::WindowStaysOnBottomHint
                          | Qt::WindowTransparentForInput
                          | Qt::WindowDoesNotAcceptFocus);
    widget.setFixedSize(1, 1);
    widget.createWinId();
//    widget.show();

    // 初始化图片解码插件，在龙芯和申威上，Qt程序冷加载图片解码插件几乎耗时1s
    Q_UNUSED(QImageReader::supportedImageFormats());

    return true;
}

int main(int argc, char **argv)
{
    QWBooster *booster = new QWBooster;

    Daemon d(argc, argv);
    d.run(booster);
}

