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


#ifndef BOOSTER_DECLARATIVE_H
#define BOOSTER_DECLARATIVE_H

#include <QScopedPointer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "booster.h"

/*!
    \class DeclarativeBooster
    \brief DeclarativeBooster is a "booster" that only exec()'s the given binary.

    This can be used to launch any application.
 */
class DeclarativeBooster : public Booster
{
public:

    DeclarativeBooster() {}
    virtual ~DeclarativeBooster() {}

    //! \reimp
    virtual const string & boosterType() const override;

    //! \reimp
    void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                    int socketFd, SingleInstance * singleInstance,
                    bool bootMode) override;

    //! Disable copy-constructor
    DeclarativeBooster(const DeclarativeBooster & r) = delete;

    //! Disable assignment operator
    DeclarativeBooster & operator= (const DeclarativeBooster & r) = delete;

protected:

    //! \reimp
    virtual bool preload() override;

private:

    static const string m_boosterType;
    QScopedPointer<QGuiApplication> m_app;
    QScopedPointer<QQmlApplicationEngine> m_engine;

    //! wait for socket connection
    void accept();
};

#endif // BOOSTER_DECLARATIVE_H
