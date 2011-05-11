#!/usr/bin/env python

# List of libraries to link against / preload.
# If linker flags are given then the library is linked againts, 
# and if the flags is None then the library will be dlopen()ed.

libraries = [
    # Library                                                           Linker flags  Dev package
    ("/usr/lib/libcontactsvoicemail.so.0",                                      None, "libcontactswidgets-dev"),
    ("/usr/lib/libmaemomeegotouchcalendarinterface.so.1",                       None, "maemo-meegotouch-interfaces-dev"),
    ("/usr/lib/libqtcontacts_extensions_tracker.so.4",                          None, "libqtcontacts-extensions-tracker-dev"),
    ("/usr/lib/libcontactsutils.so.0",                                          None, "libcontactswidgets-dev"),
    ("/usr/lib/libcontactswidgets.so.0",                                        None, "libcontactswidgets-dev"),
    ("/usr/lib/libpvr2d_r125.so",                                               None, "opengles-sgx-img-common-dev"),
    ("/usr/lib/libpvrPVR2D_DRI2WSEGL_r125.so",                                  None, "opengles-sgx-img-common-dev"),
    ("/usr/lib/libdatepicker2.so",                                              None, "libdatepicker2"),
    ("/usr/lib/libvideo-widget-meegotouch.so.1",                                None, "libvideo-widget-meegotouch-dev"),
    ("/usr/lib/libvideo-renderer-gst-playbin2.so.1",                            None, "libvideo-renderer-gst-playbin2-dev"),
    ("/usr/lib/libAccountPlugin.so.1",                                          None, "libaccountplugin-dev"),
    ("/usr/lib/libmaemomeegotouchcontactsinterface.so.1",                       None, "maemo-meegotouch-interfaces-dev"),
    ("/usr/lib/libgeoengine-mapwidget.so.1",                                    None, "geoengine-mapwidget-dev"),
    ("/usr/lib/libmcontentwidgets.so.0",                                        None, "libmcontentwidgets-dev"),
    ("/usr/lib/libQtVersit.so.1",                                               None, "libqtm-versit-dev"),
    ("/usr/lib/libMOS.so.1",                                                    None, "libmos-dev"),
    ("/usr/lib/libgallerycore.so.1",                                            None, "libgallerycore-dev"),
    ("/usr/lib/liblocationpicker.so",                                           None, "liblocationpicker"),
    ("/usr/lib/libAccountSetup.so.1",                                           None, "libaccountsetup-dev"),
    ("/usr/lib/liblocationextras.so",                                           None, "liblocationextras"),
    ("/usr/lib/libNAccountsUI.so.1",                                            None, "libnaccounts-ui-dev"),
    ("/usr/lib/libmdatauri.so.0",                                               None, "libmdatauri-dev"),
    ("/usr/lib/libgeoengine.so.1",                                              None, "geoengine-dev"),
    ("/usr/lib/libSignOnUI.so",                                                 None, "libsignon-ui0"),
    ("/usr/lib/libgq-gconf.so.0",                                               None, "libgq-gconf-dev"),
    ("/usr/lib/qt4/plugins/imageformats/libqsvg.so",                            None, "libqt4-svg"),
    ("/usr/lib/libgstinterfaces-0.10.so.0",                                     None, "libgstreamer-plugins-base0.10-dev"),
    ("/usr/lib/libgstbase-0.10.so.0",                                           None, "libgstreamer0.10-dev"),
    ("/usr/lib/qt4/plugins/imageformats/libqico.so",                            None, "libqtgui4"),
    ("/usr/lib/libquill.so.1",                                                  None, "libquill-dev"),
    ("/usr/lib/libidn.so.11",                                                   None, "libidn11-dev"),
    ("/usr/lib/libmeegocontrol.so",                                             None, "libmeegocontrol0"),
    ("/usr/lib/libcares.so.2",                                                  None, "libc-ares-dev"),
    ("/usr/lib/qt4/plugins/imageformats/libqtiff.so",                           None, "libqtgui4-dev"),
    ("/usr/lib/libgstpbutils-0.10.so.0",                                        None, "libgstreamer-plugins-base0.10-dev"),
    ("/usr/lib/libambase.so",                                                   None, "libamjpeg1"),
    ("/usr/lib/libcellular-qt.so.1",                                            None, "libcellular-qt-dev"),
    ("/usr/lib/libmoi.so.0",                                                    None, "libmoi-dev"),
    ("/usr/lib/libcurl.so.4",                                                   None, "libcurl4-openssl-dev"),
    ("/usr/lib/libcommhistory.so.0",                                            None, "libcommhistory-dev"),
    ("/usr/lib/libmessagingif0.so",                                             None, "libmessagingif0"),
    ("/usr/lib/libgstvideo-0.10.so.0",                                          None, "libgstreamer-plugins-base0.10-dev"),
    ("/usr/lib/qt4/plugins/imageformats/libqgif.so",                            None, "libqtgui4"),
    ("/usr/lib/libtiff.so.4",                                                   None, "libtiff4-dev"),
    ("/usr/lib/libresource.so",                                                 None, "libresource0"),
    ("/usr/lib/libgstreamer-0.10.so.0",                                         None, "libgstreamer0.10-dev"),
    ("/usr/lib/libQtLocation.so.1",                                             None, "libqtm-location-dev"),
    ("/usr/lib/libexempi.so.3",                                                 None, "libexempi-dev"),
    ("/usr/lib/libquillimagefilter.so.1",                                       None, "libquillimagefilter-dev"),
    ("/usr/lib/libexif.so.12",                                                  None, "libexif-dev"),
    ("/usr/lib/libresourceqt.so.1",                                             None, "libresourceqt-dev"),
    ("/usr/lib/libquillmetadata.so.1",                                          None, "libquillmetadata-dev"),
    ("/usr/lib/libprofile.so.0",                                                None, "libprofile-dev"),
    ("/usr/lib/libsaveas.so",                                                   None, "libsaveas"),
    ("/usr/lib/libsignoncrypto-qt.so.1",                                        None, "libsignoncrypto-qt-dev"),
    ("/usr/lib/libsignon-qt.so",                                                None, "libsignon-qt0"),
    ("/usr/lib/libmaemomeegotouchshareuiinterface.so.1",                        None, "maemo-meegotouch-interfaces-dev"),
    ("/usr/lib/libQtContacts.so.1",                                             None, "libqtm-contacts-dev"),
    ("/usr/lib/libduicontrolpanel.so",                                          None, "libduicontrolpanel"),
    ("/usr/lib/libaccounts-qt.so.1",                                            None, "libaccounts-qt-dev"),
    ("/usr/lib/libdbus-qeventloop.so.1",                                        None, "libdbus-qeventloop-dev"),
    ("/usr/lib/libaccounts-glib.so.0",                                          None, "libaccounts-glib-dev"),
    ("/usr/lib/libsmartsearch.so",                                              None, "libsmartsearch0"),
    ("/usr/lib/libhal.so.1",                                                    None, "libhal-dev"),
    ("/usr/lib/libcontentmanager.so.0",                                         None, "libcontentmanager-dev"),
    ("/usr/lib/librelevance.so.0",                                              None, "librelevance-dev"),
    ("/usr/lib/libdsme.so.0.2.0",                                               None, "libdsme0.2.0-dev"),
    ("/usr/lib/libtimed.so.0",                                                  None, "libtimed-dev"),
    ("/usr/lib/libqmsystem2.so.1",                                              None, "libqmsystem2-dev"),
    ("/lib/libnss_files-2.10.1.so",                                             None, "libc6"),
    ("/usr/lib/libsensordatatypes.so",                                          None, "sensord"),
    ("/usr/lib/libsensorclient.so",                                             None, "sensord"),
    ("/usr/lib/libdsme_dbus_if.so.0.2.0",                                       None, "libdsme0.2.0-dev"),
    ("/usr/lib/libqmlog.so.0",                                                  None, "libqmlog-dev"),
    ("/lib/libuuid.so.1",                                                       None, "uuid-dev"),
    ("/usr/lib/libiphb.so",                                                     None, "libiphb0"),
    ("/usr/lib/libthumbnailer.so.0",                                            None, "libthumbnailer-dev"),
    ("/usr/lib/libsysinfo.so.0",                                                None, "libsysinfo-dev"),
    ("/usr/lib/libbmeipc.so.0",                                                 None, "libbmeipc-dev"),
    ("/usr/lib/libqttracker.so.1~6",                                            None, "libqttracker-dev"),
    ("/usr/lib/libjpeg.so.62",                                                  None, "libjpeg62-dev"),
    ("/usr/lib/libQtXmlPatterns.so.4",                                          None, "libqt4-dev"),
    ("/usr/lib/libQtScript.so.4",                                               None, "libqt4-dev"),
    ("/usr/lib/libssl.so.0.9.8",                                                None, "libssl-dev"),
    ("/usr/lib/libaegis_crypto.so.1",                                           None, "libaegis-crypto-dev"),
    ("/usr/lib/libQtDeclarative.so.4",                                          None, "libqt4-dev"),
    ("/usr/lib/libxml2.so.2",                                                   None, "libxml2-dev"),
    ("/lib/libcal.so.1",                                                        None, "libcal-dev"),
    ("/usr/lib/libcontentaction.so.0",                                          None, "libcontentaction-dev"),
    ("/usr/lib/libcrypto.so.0.9.8",                                             None, "libssl-dev"),
    ("/usr/lib/libbb5.so.0",                                                    None, "libbb5-dev"),
    ("/lib/libresolv-2.10.1.so",                                                None, "libc6"),
    ("/usr/lib/libgio-2.0.so.0",                                                None, "libglib2.0-dev"),
    ("/usr/lib/libQtSparql.so.0",                                               None, "libqtsparql-dev"),
    ("/usr/lib/libsqlite3.so.0",                                                None, "libsqlite3-dev"),
    ("/usr/lib/libQtSql.so.4",                                                  None, "libqt4-dev"),
    ("/usr/lib/libmeegotouchsettings.so.0",                                     None, "libmeegotouch-dev"),
    ("/usr/lib/libmeegotouchextensions.so.0",                                   None, "libmeegotouch-dev"),
    ("/usr/lib/qt4/plugins/inputmethods/libminputcontext.so",                   None, "meego-im-context"),
    ("/usr/lib/qt4/plugins/inputmethods/libqimsw-multi.so",                     None, "libqtgui4"),
    ("/lib/libudev.so.0",                                                       None, "libudev-dev"),
    ("/usr/lib/libmeegotouchviews.so.0",                                        None, "libmeegotouch-dev"),
    ("/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so",         None, "libqt4-meegographicssystem"),
    ("/usr/lib/libXrandr.so.2",                                                 None, "libxrandr-dev"),
    ("/usr/lib/contextkit/subscriber-plugins/kbslider-1.so",                    None, "context-keyboard"),
    ("/usr/lib/libXcomposite.so.1",                                             None, "libxcomposite-dev"),
    ("/usr/lib/libGLESv2_r125.so",                                              None, "libgles2-sgx-img"),
    ("/usr/lib/libgconf-2.so.4",                                                None, "libgconf2-dev"),
    ("/usr/lib/libQtCore.so.4",                                                 None, "libqt4-dev"),
    ("/usr/lib/libdbus-glib-1.so.2",                                            None, "libdbus-glib-1-dev"),
    ("/usr/lib/libstdc++.so.6",                                                 None, "libstdc++6-4.4-dev"),
    ("/usr/lib/libXi.so.6",                                                     None, "libxi-dev"),
    ("/lib/libpcre.so.3",                                                       None, "libpcre3-dev"),
    ("/usr/lib/libQtSvg.so.4",                                                  None, "libqt4-dev"),
    ("/usr/lib/libicui18n.so.44",                                               None, "libicu-dev"),
    ("/usr/lib/libIMGegl_r125.so",                                              None, "opengles-sgx-img-common"),
    ("/usr/lib/libdbus-1.so.3",                                                 None, "libdbus-1-dev"),
    ("/usr/lib/libmeegotouchcore.so.0",                                         None, "libmeegotouch-dev"),
    ("/usr/lib/libX11.so.6",                                                    None, "libx11-dev"),
    ("/usr/lib/libexpat.so.1",                                                  None, "libexpat1-dev"),
    ("/usr/lib/libfontconfig.so.1",                                             None, "libfontconfig1-dev"),
    ("/usr/lib/libSM.so.6",                                                     None, "libsm-dev"),
    ("/usr/lib/libICE.so.6",                                                    None, "libice-dev"),
    ("/usr/lib/libEGL_r125.so",                                                 None, "opengles-sgx-img-common"),
    ("/usr/lib/libXdamage.so.1",                                                None, "libxdamage-dev"),
    ("/usr/lib/libcontextsubscriber.so.0",                                      None, "libcontextsubscriber-dev"),
    ("/lib/libdl-2.10.1.so",                                                    None, "libc6"),
    ("/usr/lib/libXext.so.6",                                                   None, "libxext-dev"),
    ("/lib/libpthread-2.10.1.so",                                               None, "libc6"),
    ("/usr/lib/libXau.so.6",                                                    None, "libxau-dev"),
    ("/usr/lib/libgobject-2.0.so.0",                                            None, "libglib2.0-dev"),
    ("/usr/lib/libcdb.so.1",                                                    None, "libcdb-dev"),
    ("/usr/lib/libQtNetwork.so.4",                                              None, "libqt4-dev"),
    ("/usr/lib/gconv/UTF-16.so",                                                None, "libc6"),
    ("/usr/lib/libicudata.so.44",                                               None, "libicu-dev"),
    ("/usr/lib/libQtMeeGoGraphicsSystemHelper.so.4",                            None, "libqt4-meegographicssystemhelper-dev"),
    ("/usr/lib/libQtGui.so.4",                                                  None, "libqt4-dev"),
    ("/usr/lib/libQtOpenGL.so.4",                                               None, "libqt4-dev"),
    ("/usr/lib/libQtXml.so.4",                                                  None, "libqt4-dev"),
    ("/usr/lib/libQtDBus.so.4",                                                 None, "libqt4-dev"),
    ("/usr/lib/libXfixes.so.3",                                                 None, "libxfixes-dev"),
    ("/usr/lib/libz.so.1",                                                      None, "zlib1g-dev"),
    ("/usr/lib/libxcb.so.1",                                                    None, "libxcb1-dev"),
    ("/lib/libm-2.10.1.so",                                                     None, "libc6"),
    ("/usr/lib/libgmodule-2.0.so.0",                                            None, "libglib2.0-dev"),
    ("/usr/lib/libsrv_um_r125.so",                                              None, "opengles-sgx-img-common"),
    ("/usr/lib/libpng12.so.0",                                                  None, "libpng12-dev"),
    ("/usr/lib/libfreetype.so.6",                                               None, "libfreetype6-dev"),
    ("/usr/lib/libgthread-2.0.so.0",                                            None, "libglib2.0-dev"),
    ("/lib/libglib-2.0.so.0",                                                   None, "libglib2.0-dev"),
    ("/usr/lib/libXrender.so.1",                                                None, "libxrender-dev"),
    ("/usr/lib/libicuuc.so.44",                                                 None, "libicu-dev"),
    ("/lib/librt-2.10.1.so",                                                    None, "libc6"),
]

import sys

if __name__ == "__main__":
    if sys.argv[1] == "--preload-h-libraries":
        f = open("preload-h-libraries.h", "w")
        f.write("// List of libraries produced by library-helper.py. DO NOT EDIT\n")
        for lib, linkerflags, devpackage in libraries:
            if not linkerflags:
                f.write('"%s",\n' % lib)
        f.close()
    elif sys.argv[1] == "--linker-flags":
        f = open("additional-linked-libraries.ld", "w")
        for lib, linkerflags, devpackage in libraries:
            if linkerflags:
                f.write('%s ' % linkerflags)
        f.close()
    elif sys.argv[1] == "--build-deps":
        packages = []
        for lib, linkerflags, devpackage in libraries:
            if linkerflags and devpackage not in packages:
                packages.append(devpackage)
        deps = ""
        for p in packages:
            deps += ", %s" % p
        print deps
    else:
        print """Usage: library-helper.py option
           Options: 
              --preload-h-libraries 
                   Produce a list of libraries in preload-h-libraries.h
              --linker-flags
                   Produdce a linker line fragment in additional-linked-libraries.ld
              --build-deps
                   Print a list of build dependencies in stdout"""

        sys.exit(1)

