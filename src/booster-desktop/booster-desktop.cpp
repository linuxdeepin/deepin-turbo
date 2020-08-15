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

#include "booster-desktop.h"
#include "daemon.h"
#include "search.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

const string DesktopBooster::m_boosterType  = "desktop";

const string & DesktopBooster::boosterType() const
{
    return m_boosterType;
}

int DesktopBooster::run(SocketManager * socketManager) {
    std::ifstream infile(m_appData->fileName());
    std::string line;
    std::string exec;
    std::string name;

    while (std::getline(infile, line)) {
        if (!std::strncmp(line.c_str(), "Exec=", 5)) {
            exec = std::move(line);
            continue;
        }

        if (!std::strncmp(line.c_str(), "Name=", 5)) {
            name = std::move(line);
            continue;
        }

        if (!exec.empty() && !name.empty()) {
            break;
        }
    }

    // remove Exec=
    exec = exec.substr(5);
    name = name.substr(5);

    std::vector<std::string> sv;
    std::istringstream iss(exec);
    std::string tmp;
    while (std::getline(iss, tmp, ' ')) {
        sv.emplace_back(std::move(tmp));
    }

    m_appData->setFileName(search_program(sv[0].c_str()));
    m_appData->setAppName(name.c_str());

    //NOTE: 需要管理内存
    char** newargv = new char*[sv.size()];
    for (size_t i = 0; i < sv.size(); ++i) {
        const std::string &str = sv[i];
        newargv[i] = new char[str.length()];
        std::strcpy(newargv[i], str.c_str());
    }

    m_appData->setArgc(sv.size());
    m_appData->setArgv(const_cast<const char**>(newargv));

    return Booster::run(socketManager);
}

bool DesktopBooster::preload()
{
    return true;
}

int main(int argc, char **argv)
{
    DesktopBooster *booster = new DesktopBooster;

    Daemon d(argc, argv);
    d.run(booster);

    return 0;
}
