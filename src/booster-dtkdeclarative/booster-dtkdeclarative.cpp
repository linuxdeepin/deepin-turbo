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
#include <DWindowManagerHelper>
#include <QImageReader>
#include <QQmlComponent>
#include <QQuickView>

DQUICK_USE_NAMESPACE

BEGIN_NAMESPACE
const string DeclarativeBooster::m_boosterType  = "dtkqml";

const string & DeclarativeBooster::boosterType() const
{
    return m_boosterType;
}

void DeclarativeBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    auto app = new QGuiApplication(initialArgc, initialArgv);
    Q_UNUSED(app) // delete in DeclarativeBooster::preload
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

    // 默认开启高分辨率图标支持
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    int argc = m_appData->argc();
    char **argv = const_cast<char**>(m_appData->argv());
    QScopedPointer<QGuiApplication> app(appLoader.createApplication(argc, argv));
    QQmlApplicationEngine engine;

    return appLoader.exec(app.data(), &engine);
}

bool DeclarativeBooster::preload()
{
    // 销毁在initialize中创建的QGuiApplication对象
    // 这个对象仅是为了preload中创建窗口使用，preload之后应当销毁它
    QScopedPointer<QGuiApplication> appClearer(qGuiApp);
    Q_UNUSED(appClearer)
    // 初始化一些必要的数据, 为后面加载的正常程序组好缓存
    QQuickView window;
    QQmlComponent component(window.engine());
    component.setData("import QtQuick.Controls 2.4\nApplicationWindow { }", QUrl());

    if (!component.create(window.rootContext())) {
        qWarning() << component.errorString();
    }

    window.create();

    // 初始化图片解码插件，在龙芯和申威上，Qt程序冷加载图片解码插件几乎耗时1s
    Q_UNUSED(QImageReader::supportedImageFormats());

    return true;
}
END_NAMESPACE

int main(int argc, char **argv)
{
    // 要保证DeclarativeBooster的析构函数能被
    // 正常执行
    DeepinTurbo::DeclarativeBooster booster;

    DeepinTurbo::Daemon d(argc, argv);
    d.run(&booster);
}
