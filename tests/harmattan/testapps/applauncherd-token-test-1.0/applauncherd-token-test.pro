## This file is part of test package applauncherd-token-test
#
# Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
#
# Contact: Marek Ruszczak <ext-marek.ruszczak@nokia.com>
#
# This software, including documentation, is protected by copyright
# controlled by Nokia Corporation. All rights are reserved. Copying,
# including reproducing, storing, adapting or translating, any or all
# of this material requires the prior written consent of Nokia
# Corporation. This material also contains confidential information
# which may not be disclosed to others without the prior written
# consent of Nokia.
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

