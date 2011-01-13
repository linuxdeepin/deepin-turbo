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

#ifndef INVOKELIB_H
#define INVOKELIB_H

#include <stdbool.h>
#include <stdint.h>

void invoke_send_msg(int fd, uint32_t msg);
void invoke_recv_msg(int fd, uint32_t *msg);

void invoke_send_str(int fd, char *str);

#define INVOKER_M_SOCK     "/tmp/boostm"
#define INVOKER_QT_SOCK    "/tmp/boostq"

#endif
