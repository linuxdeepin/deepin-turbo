/*
 * Copyright (C) 2020 ~ 2020 UnionTech Co., Ltd.
 *
 * Author:     DingYuan Zhang <justforlxz@gmail.com>
 *
 * Maintainer: DingYuan Zhang <justforlxz@gmail.com>
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

#ifndef BOOSTERDESKTOP
#define BOOSTERDESKTOP

#include "booster.h"

class DesktopBooster : public Booster
{
public:
    DesktopBooster() {}
    virtual ~DesktopBooster() {}

    //! \reimp
    virtual const string & boosterType() const override;

    virtual int run(SocketManager * socketManager) override;
    void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                    int socketFd, SingleInstance * singleInstance,
                    bool bootMode) override;

protected:
    //! \reimp
    virtual bool preload() override;

private:
    //! Disable copy-constructor
    DesktopBooster(const DesktopBooster & r);

    //! Disable assignment operator
    DesktopBooster & operator= (const DesktopBooster & r);

    static const string m_boosterType;

    //! wait for socket connection
    void accept();
};

#endif
