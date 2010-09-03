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

const char * const libs[] = { "/usr/lib/libmeegotouchcore.so.0",
                              "/usr/lib/libmeegotouchviews.so.0",
                              "/usr/lib/libmeegotouchsettings.so.0",
                              "/usr/lib/libmeegotouchextensions.so.0",
                              "/usr/lib/libQtGui.so.4",
                              "/usr/lib/libQtCore.so.4",
                              "/usr/lib/libQtCLucene.so.4",
                              "/usr/lib/libQtDBus.so.4",
                              "/usr/lib/libQtDeclarative.so.4",
                              "/usr/lib/libQtHelp.so.4",
                              "/usr/lib/libQtMultimedia.so.4",
                              "/usr/lib/libQtNetwork.so.4",
                              "/usr/lib/libQtOpenGL.so.4",
                              "/usr/lib/libQtScript.so.4",
                              "/usr/lib/libQtScriptTools.so.4",
                              "/usr/lib/libQtSql.so.4",
                              "/usr/lib/libQtSvg.so.4",
                              "/usr/lib/libQtTest.so.4",
                              "/usr/lib/libQtWebKit.so.4",
                              "/usr/lib/libQtXml.so.4",
                              "/usr/lib/libQtXmlPatterns.so.4",
                              "/usr/lib/libEGL_r125.so",
                              "/usr/lib/libGLESv2_r125.so",
                              "/usr/lib/libICE.so.6.3.0",
                              "/usr/lib/libIMGegl_r125.so",
                              "/usr/lib/libMOS.so.1.3.5",
                              "/usr/lib/libSM.so.6.0.0",
                              "/usr/lib/libX11.so.6.3.0",
                              "/usr/lib/libXau.so.6.0.0",
                              "/usr/lib/libXdamage.so.1.1.0",
                              "/usr/lib/libXdmcp.so.6.0.0",
                              "/usr/lib/libXext.so.6.4.0",
                              "/usr/lib/libXfixes.so.3.1.0",
                              "/usr/lib/libXi.so.6.1.0",
                              "/usr/lib/libXrandr.so.2.2.0",
                              "/usr/lib/libXrender.so.1.3.0",
                              "/usr/lib/libbmeipc.so.0",
                              "/usr/lib/libcdb.so.1",
                              "/usr/lib/libcellular-qt.so.1.0.0",
                              "/usr/lib/libcontentaction.so.0.0.0",
                              "/usr/lib/libcontentmanager.so.0.4.0",
                              "/usr/lib/libcontextsubscriber.so.0.0.0",
                              "/usr/lib/libcreds.so.2.0.0",
                              "/usr/lib/libcrypto.so.0.9.8",
                              "/usr/lib/libcurl.so.4.2.0",
                              "/usr/lib/libdbus-1.so.3.5.1",
                              "/usr/lib/libdbus-glib-1.so.2.1.0",
                              "/usr/lib/libdsme.so.0.2.0",
                              "/usr/lib/libdsme_dbus_if.so.0.2.0",
                              "/usr/lib/libexempi.so.3.2.1",
                              "/usr/lib/libexif.so.12.2.1",
                              "/usr/lib/libexpat.so.1.5.2",
                              "/usr/lib/libfontconfig.so.1.4.3",
                              "/usr/lib/libfreetype.so.6.6.0",
                              "/usr/lib/libgconf-2.so.4.1.0",
                              "/usr/lib/libgio-2.0.so.0.2400.1",
                              "/usr/lib/libglslcompiler_r125.so",
                              "/usr/lib/libgmodule-2.0.so.0.2400.1",
                              "/usr/lib/libgobject-2.0.so.0.2400.1",
                              "/usr/lib/libgstbase-0.10.so.0.26.0",
                              "/usr/lib/libgstinterfaces-0.10.so.0.21.0",
                              "/usr/lib/libgstreamer-0.10.so.0.26.0",
                              "/usr/lib/libgstvideo-0.10.so.0.21.0",
                              "/usr/lib/libgthread-2.0.so.0.2400.1",
                              "/usr/lib/libicudata.so.44.1",
                              "/usr/lib/libicui18n.so.44.1",
                              "/usr/lib/libicuuc.so.44.1",
                              "/usr/lib/libidn.so.11.6.1",
                              "/usr/lib/libjpeg.so.62.0.0",
                              "/usr/lib/liblocationextras.so.1.0.0",
                              "/usr/lib/liblocationpicker.so.0.3.1",
                              "/usr/lib/libmcontentwidgets.so.0.8.2",
                              "/usr/lib/libmeegotouchtheme-nokia-views.so.0.1.0",
                              "/usr/lib/libmeegotouchviews.so.0.20.37",
                              "/usr/lib/libmoi.so.0.0.0",
                              "/usr/lib/libpng12.so.0.32.0",
                              "/usr/lib/libpvr2d_r125.so",
                              "/usr/lib/libpvrPVR2D_DRI2WSEGL_r125.so",
                              "/usr/lib/libqmlog.so.0.0.4",
                              "/usr/lib/libqmsystem.so.1.0.0",
                              "/usr/lib/libqttracker.so.1~6.11.0",
                              "/usr/lib/libquill.so.1.0.0",
                              "/usr/lib/libquillimagefilter.so.1.0.0",
                              "/usr/lib/libquillmetadata.so.1.0.0",
                              "/usr/lib/librelevance.so.0.4.0",
                              "/usr/lib/libsensorclient.so.1.0.0",
                              "/usr/lib/libsensordatatypes.so.1.0.0",
                              "/usr/lib/libsmartsearch.so.0.4.0",
                              "/usr/lib/libsqlite3.so.0.8.6",
                              "/usr/lib/libsrv_um_r125.so",
                              "/usr/lib/libssl.so.0.9.8",
                              "/usr/lib/libstdc++.so.6.0.10",
                              "/usr/lib/libtimed.so.0.2.11",
                              "/usr/lib/libxcb.so.1.1.0",
                              "/usr/lib/libxml2.so.2.6.32",
                              "/usr/lib/libz.so.1.2.3.3"
                            };


#endif // PRELOAD_H
