##
# Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# Contact: directui@nokia.com
#
# This file is part of applauncherd
#
# If you have questions regarding the use of this file, please contact
# Nokia at directui@nokia.com.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation
# and appearing in the file LICENSE.LGPL included in the packaging
# of this file.
#
##

QMAKEVERSION = $$[QMAKE_VERSION]
ISQT4 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT4 ) {
    error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

QT       += core
QT       -= gui

TARGET    = fala-token-test-app
TEMPLATE  = app
CONFIG   += warn_on link_pkgconfig
CONFIG   += console
CONFIG   -= app_bundle

DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = obj

SOURCES += main.cpp

# Flags to reduce binary size where possible
QMAKE_CXXFLAGS += -fdata-sections -ffunction-sections
QMAKE_LFLAGS += --discard-local --discard-all -Wl --gc-sections
QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS += -Os

unix {
  #VARIABLES
  isEmpty(PREFIX) {
    PREFIX = /usr
  }
  BINDIR = $$PREFIX/bin
  DATADIR =$$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL

  INSTALLS += target aegisfs_config

  target.path =$$BINDIR

  aegisfs_config.files = applauncherd-token-test.conf
  aegisfs_config.path = /etc/aegisfs.d
}

