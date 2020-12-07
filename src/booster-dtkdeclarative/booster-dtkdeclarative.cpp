/*
 * Copyright (C) 2020 ~ 2020 UnionTech Co., Ltd.
 *
 * Author:     LiuYang <liuyang@uniontech.com>
 *
 * Maintainer: LiuYang <liuyang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <syslog.h>

#include "daemon.h"
#include "booster-dtkdeclarative.h"

#include <DAppLoader>

DQUICK_USE_NAMESPACE

const string DeclarativeBooster::m_boosterType  = "dtkqml";

const string & DeclarativeBooster::boosterType() const
{
    return m_boosterType;
}

void DeclarativeBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    static int argc = initialArgc;
    m_app.reset(new QGuiApplication(argc, initialArgv));
    m_engine.reset(new QQmlApplicationEngine());

    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

int DeclarativeBooster::launchProcess()
{
    setEnvironmentBeforeLaunch();

    // make booster specific initializations unless booster is in boot mode
    if (!bootMode())
        preinit();

    // Close syslog
    closelog();

    string appName = m_appData->fileName();

    appName = appName.substr(appName.find_last_of("/") + 1);
    DAppLoader appLoader(QString::fromStdString(appName));

    return appLoader.exec(m_app.data(), m_engine.data());
}

bool DeclarativeBooster::preload()
{
    return true;
}

int main(int argc, char **argv)
{
    DeclarativeBooster *booster = new DeclarativeBooster;

    Daemon d(argc, argv);
    d.run(booster);
}
