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

#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QHash>
#include <QString>

/*!
 * \class SocketManager
 *
 * SocketManager Manages sockets that are used in the invoker <-> booster
 * communication.
 */
class SocketManager
{
public:

    /*! \brief Initialize a file socket.
     *  \param socketId Path to the socket file.
     */
    void initSocket(const QString & socketId);

    /*! \brief Close a file socket.
     *  \param socketId Path to the socket file.
     */
    void closeSocket(const QString & socketId);

    //! \brief Close all open sockets.
    void closeAllSockets();

    /*! \brief Return initialized socket.
     *  \param socketId Path to the socket file.
     *  \returns socket fd or -1 on failure.
     */
    int findSocket(const QString & socketId);

    //! Return count of currently active sockets
    unsigned int socketCount() const;

private:

    typedef QHash<QString, int> SocketHash;
    SocketHash m_socketHash;
};

#endif // SOCKETMANAGER_H
