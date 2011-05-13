#!/usr/bin/env python

# List of libraries to link against / preload.
# If linker flags are given then the library is linked againts, 
# and if the flags is None then the library will be dlopen()ed.

# Real build dependencies, you need to update these if they change in debian/control
real_deps = ['cmake (>= 2.6.0)', 
             'debhelper (>= 7)', 
             'libqt4-dev (>= 4.5.0)', 
             'libmeegotouch-dev', 
             'libcreds2-dev [arm armel]', 
             'aegis-builder (>= 1.4) [arm armel]', 
             'libxtst-dev', 
             'libxext-dev', 
             'libxi-dev', 
             'doxygen', 
             'python']

# Library statuses
D = 0   # dlopen
L = 1   # link
X = 2   # omit

libraries = [
    # Library                                                           Linker flags  Dev package
    (D, "/usr/lib/libcontactsvoicemail.so.0",                              "-lcontactsvoicemail", "libcontactswidgets-dev"),
    (D, "/usr/lib/libmaemomeegotouchcalendarinterface.so.1",               "-lmaemomeegotouchcalendarinterface", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libqtcontacts_extensions_tracker.so.4",                  "-lqtcontacts_extensions_tracker", "libqtcontacts-extensions-tracker-dev"),
    (D, "/usr/lib/libcontactsutils.so.0",                                  "-lcontactsutils", "libcontactswidgets-dev"),
    (D, "/usr/lib/libcontactswidgets.so.0",                                "-lcontactswidgets", "libcontactswidgets-dev"),
    (D, "/usr/lib/libpvr2d_r125.so",                                       "-lpvr2d_r125", "opengles-sgx-img-common-dev"),
    (D, "/usr/lib/libpvrPVR2D_DRI2WSEGL_r125.so",                          "-lpvrPVR2D_DRI2WSEGL_r125", "opengles-sgx-img-common-dev"),
    (D, "/usr/lib/libdatepicker2.so",                                      "-ldatepicker2", "libdatepicker2"),
    (D, "/usr/lib/libvideo-widget-meegotouch.so.1",                        "-lvideo-widget-meegotouch", "libvideo-widget-meegotouch-dev"),
    (D, "/usr/lib/libvideo-renderer-gst-playbin2.so.1",                    "-lvideo-renderer-gst-playbin2", "libvideo-renderer-gst-playbin2-dev"),
    (D, "/usr/lib/libAccountPlugin.so.1",                                  "-lAccountPlugin", "libaccountplugin-dev"),
    (D, "/usr/lib/libmaemomeegotouchcontactsinterface.so.1",               "-lmaemomeegotouchcontactsinterface", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libgeoengine-mapwidget.so.1",                            "-lgeoengine-mapwidget", "geoengine-mapwidget-dev"),
    (D, "/usr/lib/libmcontentwidgets.so.0",                                "-lmcontentwidgets", "libmcontentwidgets-dev"),
    (D, "/usr/lib/libQtVersit.so.1",                                       "-lQtVersit", "libqtm-versit-dev"),
    (D, "/usr/lib/libMOS.so.1",                                            "-lMOS", "libmos-dev"),
    (D, "/usr/lib/libgallerycore.so.1",                                    "-lgallerycore", "libgallerycore-dev"),
    (D, "/usr/lib/liblocationpicker.so",                                   "-llocationpicker", "liblocationpicker"),
    (D, "/usr/lib/libAccountSetup.so.1",                                   "-lAccountSetup", "libaccountsetup-dev"),
    (D, "/usr/lib/liblocationextras.so",                                   "-llocationextras", "liblocationextras"),
    (D, "/usr/lib/libNAccountsUI.so.1",                                    "-lNAccountsUI", "libnaccounts-ui-dev"),
    (D, "/usr/lib/libmdatauri.so.0",                                       "-lmdatauri", "libmdatauri-dev"),
    (D, "/usr/lib/libgeoengine.so.1",                                      "-lgeoengine", "geoengine-dev"),
    (D, "/usr/lib/libSignOnUI.so",                                         "-lSignOnUI", "libsignon-ui0"),
    (D, "/usr/lib/libgq-gconf.so.0",                                       "-lgq-gconf", "libgq-gconf-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqsvg.so",                    "/usr/lib/qt4/plugins/imageformats/libqsvg.so", "libqt4-svg"),
    (D, "/usr/lib/libgstinterfaces-0.10.so.0",                             "-lgstinterfaces-0.10", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/libgstbase-0.10.so.0",                                   "-lgstbase-0.10", "libgstreamer0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqico.so",                    "/usr/lib/qt4/plugins/imageformats/libqico.so", "libqtgui4"),
    (D, "/usr/lib/libquill.so.1",                                          "-lquill", "libquill-dev"),
    (D, "/usr/lib/libidn.so.11",                                           "-lidn", "libidn11-dev"),
    (D, "/usr/lib/libmeegocontrol.so",                                     "-lmeegocontrol", "libmeegocontrol0"),
    (D, "/usr/lib/libcares.so.2",                                          "-lcares", "libc-ares-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqtiff.so",                   "/usr/lib/qt4/plugins/imageformats/libqtiff.so", "libqtgui4-dev"),
    (D, "/usr/lib/libgstpbutils-0.10.so.0",                                "-lgstpbutils-0.10", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/libambase.so",                                           "-lambase", "libamjpeg1"),
    (D, "/usr/lib/libcellular-qt.so.1",                                    "-lcellular-qt", "libcellular-qt-dev"),
    (D, "/usr/lib/libmoi.so.0",                                            "-lmoi", "libmoi-dev"),
    (D, "/usr/lib/libcurl.so.4",                                           "-lcurl", "libcurl4-openssl-dev"),
    (D, "/usr/lib/libcommhistory.so.0",                                    "-lcommhistory", "libcommhistory-dev"),
    (D, "/usr/lib/libmessagingif0.so",                                     "-lmessagingif0", "libmessagingif0"),
    (D, "/usr/lib/libgstvideo-0.10.so.0",                                  "-lgstvideo-0.10", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqgif.so",                    "/usr/lib/qt4/plugins/imageformats/libqgif.so", "libqtgui4"),
    (D, "/usr/lib/libtiff.so.4",                                           "-ltiff", "libtiff4-dev"),
    (D, "/usr/lib/libresource.so",                                         "-lresource", "libresource0"),
    (D, "/usr/lib/libgstreamer-0.10.so.0",                                 "-lgstreamer-0.10", "libgstreamer0.10-dev"),
    (D, "/usr/lib/libQtLocation.so.1",                                     "-lQtLocation", "libqtm-location-dev"),
    (D, "/usr/lib/libexempi.so.3",                                         "-lexempi", "libexempi-dev"),
    (D, "/usr/lib/libquillimagefilter.so.1",                               "-lquillimagefilter", "libquillimagefilter-dev"),
    (D, "/usr/lib/libexif.so.12",                                          "-lexif", "libexif-dev"),
    (D, "/usr/lib/libresourceqt.so.1",                                     "-lresourceqt", "libresourceqt-dev"),
    (D, "/usr/lib/libquillmetadata.so.1",                                  "-lquillmetadata", "libquillmetadata-dev"),
    (D, "/usr/lib/libprofile.so.0",                                        "-lprofile", "libprofile-dev"),
    (D, "/usr/lib/libsaveas.so",                                           "-lsaveas", "libsaveas"),
    (D, "/usr/lib/libsignoncrypto-qt.so.1",                                "-lsignoncrypto-qt", "libsignoncrypto-qt-dev"),
    (D, "/usr/lib/libsignon-qt.so",                                        "-lsignon-qt", "libsignon-qt0"),
    (D, "/usr/lib/libmaemomeegotouchshareuiinterface.so.1",                "-lmaemomeegotouchshareuiinterface", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libQtContacts.so.1",                                     "-lQtContacts", "libqtm-contacts-dev"),
    (D, "/usr/lib/libduicontrolpanel.so",                                  "-lduicontrolpanel", "libduicontrolpanel"),
    (D, "/usr/lib/libaccounts-qt.so.1",                                    "-laccounts-qt", "libaccounts-qt-dev"),
    (D, "/usr/lib/libdbus-qeventloop.so.1",                                "-ldbus-qeventloop", "libdbus-qeventloop-dev"),
    (D, "/usr/lib/libaccounts-glib.so.0",                                  "-laccounts-glib", "libaccounts-glib-dev"),
    (D, "/usr/lib/libsmartsearch.so",                                      "-lsmartsearch", "libsmartsearch0"),
    (D, "/usr/lib/libhal.so.1",                                            "-lhal", "libhal-dev"),
    (D, "/usr/lib/libcontentmanager.so.0",                                 "-lcontentmanager", "libcontentmanager-dev"),
    (D, "/usr/lib/librelevance.so.0",                                      "-lrelevance", "librelevance-dev"),
    (D, "/usr/lib/libdsme.so.0.2.0",                                       "-ldsme", "libdsme0.2.0-dev"),
    (D, "/usr/lib/libtimed.so.0",                                          "-ltimed", "libtimed-dev"),
    (D, "/usr/lib/libqmsystem2.so.1",                                      "-lqmsystem2", "libqmsystem2-dev"),
    (D, "/lib/libnss_files-2.10.1.so",                                     "-lnss_files-2.10.1", "libc6"),
    (D, "/usr/lib/libsensordatatypes.so",                                  "-lsensordatatypes", "sensord"),
    (D, "/usr/lib/libsensorclient.so",                                     "-lsensorclient", "sensord"),
    (D, "/usr/lib/libdsme_dbus_if.so.0.2.0",                               "-ldsme_dbus_if", "libdsme0.2.0-dev"),
    (D, "/usr/lib/libqmlog.so.0",                                          "-lqmlog", "libqmlog-dev"),
    (D, "/lib/libuuid.so.1",                                               "-luuid", "uuid-dev"),
    (D, "/usr/lib/libiphb.so",                                             "-liphb", "libiphb0"),
    (D, "/usr/lib/libthumbnailer.so.0",                                    "-lthumbnailer", "libthumbnailer-dev"),
    (D, "/usr/lib/libsysinfo.so.0",                                        "-lsysinfo", "libsysinfo-dev"),
    (D, "/usr/lib/libbmeipc.so.0",                                         "-lbmeipc", "libbmeipc-dev"),
    (D, "/usr/lib/libqttracker.so.1~6",                                    "-lqttracker", "libqttracker-dev"),
    (D, "/usr/lib/libjpeg.so.62",                                          "-ljpeg", "libjpeg62-dev"),
    (D, "/usr/lib/libQtXmlPatterns.so.4",                                  "-lQtXmlPatterns", "libqt4-dev"),
    (D, "/usr/lib/libQtScript.so.4",                                       "-lQtScript", "libqt4-dev"),
    (D, "/usr/lib/libssl.so.0.9.8",                                        "-lssl", "libssl-dev"),
    (D, "/usr/lib/libaegis_crypto.so.1",                                   "-laegis_crypto", "libaegis-crypto-dev"),
    (D, "/usr/lib/libQtDeclarative.so.4",                                  "-lQtDeclarative", "libqt4-dev"),
    (D, "/usr/lib/libxml2.so.2",                                           "-lxml2", "libxml2-dev"),
    (D, "/lib/libcal.so.1",                                                "-lcal", "libcal-dev"),
    (D, "/usr/lib/libcontentaction.so.0",                                  "-lcontentaction", "libcontentaction-dev"),
    (D, "/usr/lib/libcrypto.so.0.9.8",                                     "-lcrypto", "libssl-dev"),
    (D, "/usr/lib/libbb5.so.0",                                            "-lbb5", "libbb5-dev"),
    (D, "/lib/libresolv-2.10.1.so",                                        "-lresolv-2.10.1", "libc6"),
    (D, "/usr/lib/libgio-2.0.so.0",                                        "-lgio-2.0", "libglib2.0-dev"),
    (D, "/usr/lib/libQtSparql.so.0",                                       "-lQtSparql", "libqtsparql-dev"),
    (D, "/usr/lib/libsqlite3.so.0",                                        "-lsqlite3", "libsqlite3-dev"),
    (D, "/usr/lib/libQtSql.so.4",                                          "-lQtSql", "libqt4-dev"),
    (D, "/usr/lib/libmeegotouchsettings.so.0",                             "-lmeegotouchsettings", "libmeegotouch-dev"),
    (D, "/usr/lib/libmeegotouchextensions.so.0",                           "-lmeegotouchextensions", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libminputcontext.so",           "-lminputcontext", "meego-im-context"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libqimsw-multi.so",             "-lqimsw-multi", "libqtgui4"),
    (D, "/lib/libudev.so.0",                                               "-ludev", "libudev-dev"),
    (D, "/usr/lib/libmeegotouchviews.so.0",                                "-lmeegotouchviews", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "libqt4-meegographicssystem"),
    (D, "/usr/lib/libXrandr.so.2",                                         "-lXrandr", "libxrandr-dev"),
    (D, "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so",            "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so", "context-keyboard"),
    (D, "/usr/lib/libXcomposite.so.1",                                     "-lXcomposite", "libxcomposite-dev"),
    (D, "/usr/lib/libGLESv2_r125.so",                                      "-lGLESv2_r125", "libgles2-sgx-img"),
    (D, "/usr/lib/libgconf-2.so.4",                                        "-lgconf-2", "libgconf2-dev"),
    (D, "/usr/lib/libQtCore.so.4",                                         "-lQtCore", "libqt4-dev"),
    (D, "/usr/lib/libdbus-glib-1.so.2",                                    "-ldbus-glib-1", "libdbus-glib-1-dev"),
    (D, "/usr/lib/libstdc++.so.6",                                         "-lstdc++", "libstdc++6-4.4-dev"),
    (D, "/usr/lib/libXi.so.6",                                             "-lXi", "libxi-dev"),
    (D, "/lib/libpcre.so.3",                                               "-lpcre", "libpcre3-dev"),
    (D, "/usr/lib/libQtSvg.so.4",                                          "-lQtSvg", "libqt4-dev"),
    (D, "/usr/lib/libicui18n.so.44",                                       "-licui18n", "libicu-dev"),
    (D, "/usr/lib/libIMGegl_r125.so",                                      "-lIMGegl_r125", "opengles-sgx-img-common"),
    (D, "/usr/lib/libdbus-1.so.3",                                         "-ldbus-1", "libdbus-1-dev"),
    (D, "/usr/lib/libmeegotouchcore.so.0",                                 "-lmeegotouchcore", "libmeegotouch-dev"),
    (D, "/usr/lib/libX11.so.6",                                            "-lX11", "libx11-dev"),
    (D, "/usr/lib/libexpat.so.1",                                          "-lexpat", "libexpat1-dev"),
    (D, "/usr/lib/libfontconfig.so.1",                                     "-lfontconfig", "libfontconfig1-dev"),
    (D, "/usr/lib/libSM.so.6",                                             "-lSM", "libsm-dev"),
    (D, "/usr/lib/libICE.so.6",                                            "-lICE", "libice-dev"),
    (D, "/usr/lib/libEGL_r125.so",                                         "-lEGL_r125", "opengles-sgx-img-common"),
    (D, "/usr/lib/libXdamage.so.1",                                        "-lXdamage", "libxdamage-dev"),
    (D, "/usr/lib/libcontextsubscriber.so.0",                              "-lcontextsubscriber", "libcontextsubscriber-dev"),
    (D, "/lib/libdl-2.10.1.so",                                            "-ldl-2.10.1", "libc6"),
    (D, "/usr/lib/libXext.so.6",                                           "-lXext", "libxext-dev"),
    (D, "/lib/libpthread-2.10.1.so",                                       "-lpthread-2.10.1", "libc6"),
    (D, "/usr/lib/libXau.so.6",                                            "-lXau", "libxau-dev"),
    (D, "/usr/lib/libgobject-2.0.so.0",                                    "-lgobject-2.0", "libglib2.0-dev"),
    (D, "/usr/lib/libcdb.so.1",                                            "-lcdb", "libcdb-dev"),
    (D, "/usr/lib/libQtNetwork.so.4",                                      "-lQtNetwork", "libqt4-dev"),
    (D, "/usr/lib/gconv/UTF-16.so",                                        "/usr/lib/gconv/UTF-16.so", "libc6"),
    (D, "/usr/lib/libicudata.so.44",                                       "-licudata", "libicu-dev"),
    (D, "/usr/lib/libQtMeeGoGraphicsSystemHelper.so.4",                    "-lQtMeeGoGraphicsSystemHelper", "libqt4-meegographicssystemhelper-dev"),
    (D, "/usr/lib/libQtGui.so.4",                                          "-lQtGui", "libqt4-dev"),
    (D, "/usr/lib/libQtOpenGL.so.4",                                       "-lQtOpenGL", "libqt4-dev"),
    (D, "/usr/lib/libQtXml.so.4",                                          "-lQtXml", "libqt4-dev"),
    (D, "/usr/lib/libQtDBus.so.4",                                         "-lQtDBus", "libqt4-dev"),
    (D, "/usr/lib/libXfixes.so.3",                                         "-lXfixes", "libxfixes-dev"),
    (D, "/usr/lib/libz.so.1",                                              "-lz", "zlib1g-dev"),
    (D, "/usr/lib/libxcb.so.1",                                            "-lxcb", "libxcb1-dev"),
    (D, "/lib/libm-2.10.1.so",                                             "-lm-2.10.1", "libc6"),
    (D, "/usr/lib/libgmodule-2.0.so.0",                                    "-lgmodule-2.0", "libglib2.0-dev"),
    (D, "/usr/lib/libsrv_um_r125.so",                                      "-lsrv_um_r125", "opengles-sgx-img-common"),
    (D, "/usr/lib/libpng12.so.0",                                          "-lpng12", "libpng12-dev"),
    (D, "/usr/lib/libfreetype.so.6",                                       "-lfreetype", "libfreetype6-dev"),
    (D, "/usr/lib/libgthread-2.0.so.0",                                    "-lgthread-2.0", "libglib2.0-dev"),
    (D, "/lib/libglib-2.0.so.0",                                           "-lglib-2.0", "libglib2.0-dev"),
    (D, "/usr/lib/libXrender.so.1",                                        "-lXrender", "libxrender-dev"),
    (D, "/usr/lib/libicuuc.so.44",                                         "-licuuc", "libicu-dev"),
    (D, "/lib/librt-2.10.1.so",                                            "-lrt-2.10.1", "libc6"),
]

import sys
import fileinput
import re

def build_deps_from_debian_control(control_file):
    res = set()
    dep_reg = re.compile(r"^Build-Depends:(.*)$")
    for line in fileinput.input(control_file):
        m = dep_reg.match(line)
        if m:
            for dep in m.group(1).split(","):
                res.add(dep.strip())
            return res

if __name__ == "__main__":
    if sys.argv[1] == "--preload-h-libraries":
        f = open("preload-h-libraries.h", "w")
        f.write("// List of libraries produced by library-helper.py. DO NOT EDIT\n")
        for status, lib, linkerflags, devpackage in libraries:
            if status == D:
                f.write('"%s",\n' % lib)
        f.close()
    elif sys.argv[1] == "--linker-flags":
        f = open("additional-linked-libraries.ld", "w")
        for status, lib, linkerflags, devpackage in libraries:
            if status == L:
                f.write('%s ' % linkerflags)
        f.close()
    elif sys.argv[1] == "--check-build-depends":
        all_packages = set()
        dep_packages = set()
        for status, lib, linkerflags, devpackage in libraries:
            all_packages.add(devpackage)
            if status == L:
                dep_packages.add(devpackage)
        build_deps = build_deps_from_debian_control(sys.argv[2])
        for p in all_packages:
            if p in dep_packages and p not in build_deps:
                print "Package missing from Build-Depends: %s" % p
        for p in build_deps:
            if p not in dep_packages and p not in real_deps:
                print "Package redundant in Build-Depends: %s" % p
        for p in real_deps:
            if p not in build_deps:
                print "Real dependency missing in Build-Depends: %s" % p
    else:
        print """Usage: library-helper.py option
           Options: 
              --preload-h-libraries 
                   Produce a list of libraries in preload-h-libraries.h
              --linker-flags
                   Produdce a linker line fragment in additional-linked-libraries.ld
              --check-build-depends control-file
                   Given the debian control file, check if there are missing packages
                   or redundant packages in Build-Depends.
"""

        sys.exit(1)

