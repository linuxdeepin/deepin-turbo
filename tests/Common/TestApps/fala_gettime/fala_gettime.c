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

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
  struct timeval tim;
  gettimeofday(&tim, NULL);
  printf("%d.%06d\n", (int)(tim.tv_sec), (int)(tim.tv_usec));
  return EXIT_SUCCESS;
}

