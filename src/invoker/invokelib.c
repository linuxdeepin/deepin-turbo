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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "report.h"
#include "invokelib.h"

bool invoke_send_msg(int fd, uint32_t msg)
{
    debug("%s: %08x\n", __FUNCTION__, msg);
    return write(fd, &msg, sizeof(msg)) != -1;
}

bool invoke_recv_msg(int fd, uint32_t *msg)
{
    int res = read(fd, msg, sizeof(*msg));
    debug("%s: %08x\n", __FUNCTION__, *msg);
    return res != -1;
}

bool invoke_send_str(int fd, char *str)
{
    uint32_t size;

    /* Send size. */
    size = strlen(str) + 1;
    invoke_send_msg(fd, size);

    debug("%s: '%s'\n", __FUNCTION__, str);

    /* Send the string. */
    return write(fd, str, size) != -1;
}

