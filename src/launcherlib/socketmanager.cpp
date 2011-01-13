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

#include "socketmanager.h"
#include "logger.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

void SocketManager::initSocket(const string & socketId)
{
    // Initialize a socket at socketId if one already doesn't
    // exist for that id / path.
    if (m_socketHash.find(socketId) == m_socketHash.end())
    {
        Logger::logDebug("SoketManager: Initing socket at '%s'..", socketId.c_str());

        // Create a new local socket
        int socketFd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (socketFd < 0)
            Logger::logErrorAndDie(EXIT_FAILURE, "SocketManager: Failed to open socket\n");

        // TODO: Error if socketId >= maxLen. Also unlink() here may
        // try to remove a different file than is passed to sun.sa_data.

        // Remove the previous socket file
        unlink(socketId.c_str());

        // Initialize the socket struct
        struct sockaddr sun;
        sun.sa_family = AF_UNIX;
        int maxLen = sizeof(sun.sa_data) - 1;
        strncpy(sun.sa_data, socketId.c_str(), maxLen);
        sun.sa_data[maxLen] = '\0';

        // Bind the socket
        if (bind(socketFd, &sun, sizeof(sun)) < 0)
            Logger::logErrorAndDie(EXIT_FAILURE, "SocketManager: Failed to bind to socket (fd=%d)\n", socketFd);

        // Listen to the socket
        if (listen(socketFd, 10) < 0)
            Logger::logErrorAndDie(EXIT_FAILURE, "SocketManager: Failed to listen to socket (fd=%d)\n", socketFd);

        // Set permissions
        chmod(socketId.c_str(), S_IRUSR | S_IWUSR | S_IXUSR |
              S_IRGRP | S_IWGRP | S_IXGRP |
              S_IROTH | S_IWOTH | S_IXOTH);

        // Store path <-> file descriptor mapping
        m_socketHash[socketId] = socketFd;
    }
}

void SocketManager::closeSocket(const string & socketId)
{
    SocketHash::iterator it(m_socketHash.find(socketId));

    if (it != m_socketHash.end())
    {
        ::close(it->second);
        m_socketHash.erase(it);
    }
}

void SocketManager::closeAllSockets()
{
    SocketHash::iterator it;
    for (it = m_socketHash.begin(); it != m_socketHash.end(); ++it)
    {
        if (it->second > 0)
        {
            ::close(it->second);
        }
    }

    m_socketHash.clear();
}

int SocketManager::findSocket(const string & socketId)
{
    SocketHash::iterator i(m_socketHash.find(socketId));
    return i == m_socketHash.end() ? -1 : i->second;
}

unsigned int SocketManager::socketCount() const
{
    return m_socketHash.size();
}

