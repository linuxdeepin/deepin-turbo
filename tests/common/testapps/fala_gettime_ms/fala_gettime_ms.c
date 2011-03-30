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

int main(int argc, char **argv) {
  struct timeval tim;
  gettimeofday(&tim, NULL);
  if (argc > 0) {
      printf("%d%03d %s\n", 
             (int)(tim.tv_sec), (int)(tim.tv_usec/1000), argv[1]);
  } else {
      printf("%d%03d\n", (int)(tim.tv_sec), (int)(tim.tv_usec/1000));
  }
  return EXIT_SUCCESS;
}

