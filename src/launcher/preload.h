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

#ifndef PRELOAD_H
#define PRELOAD_H

const char * const gLibs[] =
{

    // Include the generated set of libraries.
#include "preload-h-libraries.h"

    "/usr/bin/single-instance",
    // Library to trigger LMT font and style initializations
    "/usr/lib/libmeegotouchpreloader.so.0"
};

#endif // PRELOAD_H
