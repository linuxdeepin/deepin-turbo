#!/usr/bin/env python

# List of libraries to link against / preload.
# If linker flags are given then the library is linked againts, 
# and if the flags is None then the library will be dlopen()ed.

# Real build dependencies, you need to update these if they change in debian/control
real_build_deps = set(['cmake (>= 2.6.0)', 
                       'debhelper (>= 7)', 
                       'libqt4-dev (>= 4.5.0)', 
                       'libmeegotouch-dev', 
                       'libcreds2-dev [arm armel]', 
                       'aegis-builder (>= 1.4) [arm armel]', 
                       'libxtst-dev', 
                       'libxext-dev', 
                       'libxi-dev', 
                       'doxygen', 
                       'python'])

# Real runtime dependencies for applauncherd-launcher, you may need to update these as well
real_runtime_deps = set(['${shlibs:Depends}', 
                         '${misc:Depends}', 
                         'libcreds2 [arm armel]', 
                         'applauncherd-single-instance'])

# Library statuses
D = 0   # dlopen
L = 1   # link

libraries = [
    # Library                                                              Linker flags, Binary package, Dev package
    (D, "/usr/lib/libcontactsvoicemail.so.0",                              "-lcontactsvoicemail", "libcontactsvoicemail0", "libcontactswidgets-dev"),
    (D, "/usr/lib/libmaemomeegotouchcalendarinterface.so.1",               "-lmaemomeegotouchcalendarinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libqtcontacts_extensions_tracker.so.4",                  "-lqtcontacts_extensions_tracker", "libqtcontacts-extensions-tracker0", "libqtcontacts-extensions-tracker-dev"),
    (D, "/usr/lib/libcontactsutils.so.0",                                  "-lcontactsutils", "libcontactswidgets0", "libcontactswidgets-dev"),
    (D, "/usr/lib/libcontactswidgets.so.0",                                "-lcontactswidgets", "libcontactswidgets0", "libcontactswidgets-dev"),
    (D, "/usr/lib/libpvr2d_r125.so",                                       "-lpvr2d_r125", "opengles-sgx-img-common", "opengles-sgx-img-common-dev"),
    (D, "/usr/lib/libpvrPVR2D_DRI2WSEGL_r125.so",                          "-lpvrPVR2D_DRI2WSEGL_r125", "opengles-sgx-img-common", "opengles-sgx-img-common-dev"),
    (D, "/usr/lib/libdatepicker2.so",                                      "-ldatepicker2", "libdatepicker2", "libdatepicker2"),
    (D, "/usr/lib/libvideo-widget-meegotouch.so.1",                        "-lvideo-widget-meegotouch", "libvideo-widget-meegotouch", "libvideo-widget-meegotouch-dev"),
    (D, "/usr/lib/libvideo-renderer-gst-playbin2.so.1",                    "-lvideo-renderer-gst-playbin2", "libvideo-renderer-gst-playbin2", "libvideo-renderer-gst-playbin2-dev"),
    (D, "/usr/lib/libAccountPlugin.so.1",                                  "-lAccountPlugin", "libaccountplugin0", "libaccountplugin-dev"),
    (D, "/usr/lib/libmaemomeegotouchcontactsinterface.so.1",               "-lmaemomeegotouchcontactsinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libgeoengine-mapwidget.so.1",                            "-lgeoengine-mapwidget", "geoengine-mapwidget", "geoengine-mapwidget-dev"),
    (D, "/usr/lib/libmcontentwidgets.so.0",                                "-lmcontentwidgets", "libmcontentwidgets0", "libmcontentwidgets-dev"),
    (D, "/usr/lib/libQtVersit.so.1",                                       "-lQtVersit", "libqtm-versit", "libqtm-versit-dev"),
    (D, "/usr/lib/libMOS.so.1",                                            "-lMOS", "libmos", "libmos-dev"),
    (D, "/usr/lib/libgallerycore.so.1",                                    "-lgallerycore", "libgallerycore", "libgallerycore-dev"),
    (D, "/usr/lib/liblocationpicker.so",                                   "-llocationpicker", "liblocationpicker", "liblocationpicker"),
    (D, "/usr/lib/libAccountSetup.so.1",                                   "-lAccountSetup", "libaccountsetup0", "libaccountsetup-dev"),
    (D, "/usr/lib/liblocationextras.so",                                   "-llocationextras", "liblocationextras", "liblocationextras"),
    (D, "/usr/lib/libNAccountsUI.so.1",                                    "-lNAccountsUI", "libnaccounts-ui0", "libnaccounts-ui-dev"),
    (D, "/usr/lib/libmdatauri.so.0",                                       "-lmdatauri", "libmdatauri0", "libmdatauri-dev"),
    (D, "/usr/lib/libgeoengine.so.1",                                      "-lgeoengine", "geoengine", "geoengine-dev"),
    (D, "/usr/lib/libSignOnUI.so",                                         "-lSignOnUI", "libsignon-ui0", "libsignon-ui0"),
    (D, "/usr/lib/libgq-gconf.so.0",                                       "-lgq-gconf", "libgq-gconf0", "libgq-gconf-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqsvg.so",                    "/usr/lib/qt4/plugins/imageformats/libqsvg.so", "libqt4-svg", "libqt4-svg"),
    (D, "/usr/lib/libgstinterfaces-0.10.so.0",                             "-lgstinterfaces-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/libgstbase-0.10.so.0",                                   "-lgstbase-0.10", "libgstreamer0.10-0", "libgstreamer0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqico.so",                    "/usr/lib/qt4/plugins/imageformats/libqico.so", "libqtgui4", "libqtgui4"),
    (D, "/usr/lib/libquill.so.1",                                          "-lquill", "libquill1", "libquill-dev"),
    (D, "/usr/lib/libidn.so.11",                                           "-lidn", "libidn11", "libidn11-dev"),
    (D, "/usr/lib/libmeegocontrol.so",                                     "-lmeegocontrol", "libmeegocontrol0", "libmeegocontrol0"),
    (D, "/usr/lib/libcares.so.2",                                          "-lcares", "libc-ares2", "libc-ares-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqtiff.so",                   "/usr/lib/qt4/plugins/imageformats/libqtiff.so", "libqtgui4", "libqtgui4-dev"),
    (D, "/usr/lib/libgstpbutils-0.10.so.0",                                "-lgstpbutils-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/libambase.so",                                           "-lambase", "libamjpeg1", "libamjpeg1"),
    (D, "/usr/lib/libcellular-qt.so.1",                                    "-lcellular-qt", "libcellular-qt1", "libcellular-qt-dev"),
    (D, "/usr/lib/libmoi.so.0",                                            "-lmoi", "libmoi0", "libmoi-dev"),
    (D, "/usr/lib/libcurl.so.4",                                           "-lcurl", "libcurl3", "libcurl4-openssl-dev"),
    (D, "/usr/lib/libcommhistory.so.0",                                    "-lcommhistory", "libcommhistory0", "libcommhistory-dev"),
    (D, "/usr/lib/libmessagingif0.so",                                     "-lmessagingif0", "libmessagingif0", "libmessagingif0"),
    (D, "/usr/lib/libgstvideo-0.10.so.0",                                  "-lgstvideo-0.10", "libgstreamer-plugins-base0.10-0", "libgstreamer-plugins-base0.10-dev"),
    (D, "/usr/lib/qt4/plugins/imageformats/libqgif.so",                    "/usr/lib/qt4/plugins/imageformats/libqgif.so", "libqtgui4", "libqtgui4"),
    (D, "/usr/lib/libtiff.so.4",                                           "-ltiff", "libtiff4", "libtiff4-dev"),
    (D, "/usr/lib/libresource.so",                                         "-lresource", "libresource0", "libresource0"),
    (D, "/usr/lib/libgstreamer-0.10.so.0",                                 "-lgstreamer-0.10", "libgstreamer0.10-0", "libgstreamer0.10-dev"),
    (D, "/usr/lib/libQtLocation.so.1",                                     "-lQtLocation", "libqtm-location", "libqtm-location-dev"),
    (D, "/usr/lib/libexempi.so.3",                                         "-lexempi", "libexempi3", "libexempi-dev"),
    (D, "/usr/lib/libquillimagefilter.so.1",                               "-lquillimagefilter", "libquillimagefilter1", "libquillimagefilter-dev"),
    (D, "/usr/lib/libexif.so.12",                                          "-lexif", "libexif12", "libexif-dev"),
    (D, "/usr/lib/libresourceqt.so.1",                                     "-lresourceqt", "libresourceqt1", "libresourceqt-dev"),
    (D, "/usr/lib/libquillmetadata.so.1",                                  "-lquillmetadata", "libquillmetadata1", "libquillmetadata-dev"),
    (D, "/usr/lib/libprofile.so.0",                                        "-lprofile", "libprofile0", "libprofile-dev"),
    (D, "/usr/lib/libsaveas.so",                                           "-lsaveas", "libsaveas", "libsaveas"),
    (D, "/usr/lib/libsignoncrypto-qt.so.1",                                "-lsignoncrypto-qt", "libsignoncrypto-qt", "libsignoncrypto-qt-dev"),
    (D, "/usr/lib/libsignon-qt.so",                                        "-lsignon-qt", "libsignon-qt0", "libsignon-qt0"),
    (D, "/usr/lib/libmaemomeegotouchshareuiinterface.so.1",                "-lmaemomeegotouchshareuiinterface", "maemo-meegotouch-interfaces", "maemo-meegotouch-interfaces-dev"),
    (D, "/usr/lib/libQtContacts.so.1",                                     "-lQtContacts", "libqtm-contacts", "libqtm-contacts-dev"),
    (D, "/usr/lib/libduicontrolpanel.so",                                  "-lduicontrolpanel", "libduicontrolpanel", "libduicontrolpanel"),
    (D, "/usr/lib/libaccounts-qt.so.1",                                    "-laccounts-qt", "libaccounts-qt1", "libaccounts-qt-dev"),
    (D, "/usr/lib/libdbus-qeventloop.so.1",                                "-ldbus-qeventloop", "libdbus-qeventloop1", "libdbus-qeventloop-dev"),
    (D, "/usr/lib/libaccounts-glib.so.0",                                  "-laccounts-glib", "libaccounts-glib0", "libaccounts-glib-dev"),
    (D, "/usr/lib/libsmartsearch.so",                                      "-lsmartsearch", "libsmartsearch0", "libsmartsearch0"),
    (D, "/usr/lib/libhal.so.1",                                            "-lhal", "libhal1", "libhal-dev"),
    (D, "/usr/lib/libcontentmanager.so.0",                                 "-lcontentmanager", "libcontentmanager0", "libcontentmanager-dev"),
    (D, "/usr/lib/librelevance.so.0",                                      "-lrelevance", "librelevance0", "librelevance-dev"),
    (D, "/usr/lib/libdsme.so.0.2.0",                                       "-ldsme", "libdsme0.2.0", "libdsme0.2.0-dev"),
    (D, "/usr/lib/libtimed.so.0",                                          "-ltimed", "libtimed0", "libtimed-dev"),
    (D, "/usr/lib/libqmsystem2.so.1",                                      "-lqmsystem2", "libqmsystem2", "libqmsystem2-dev"),
    (D, "/lib/libnss_files-2.10.1.so",                                     "-lnss_files-2.10.1", "libc6", "libc6"),
    (D, "/usr/lib/libsensordatatypes.so",                                  "-lsensordatatypes", "sensord", "sensord"),
    (D, "/usr/lib/libsensorclient.so",                                     "-lsensorclient", "sensord", "sensord"),
    (D, "/usr/lib/libdsme_dbus_if.so.0.2.0",                               "-ldsme_dbus_if", "libdsme0.2.0", "libdsme0.2.0-dev"),
    (D, "/usr/lib/libqmlog.so.0",                                          "-lqmlog", "libqmlog0", "libqmlog-dev"),
    (D, "/lib/libuuid.so.1",                                               "-luuid", "libuuid1", "uuid-dev"),
    (D, "/usr/lib/libiphb.so",                                             "-liphb", "libiphb0", "libiphb0"),
    (D, "/usr/lib/libthumbnailer.so.0",                                    "-lthumbnailer", "libthumbnailer0", "libthumbnailer-dev"),
    (D, "/usr/lib/libsysinfo.so.0",                                        "-lsysinfo", "libsysinfo0", "libsysinfo-dev"),
    (D, "/usr/lib/libbmeipc.so.0",                                         "-lbmeipc", "libbmeipc0", "libbmeipc-dev"),
    (D, "/usr/lib/libqttracker.so.1~6",                                    "-lqttracker", "libqttracker1pre6", "libqttracker-dev"),
    (D, "/usr/lib/libjpeg.so.62",                                          "-ljpeg", "libjpeg62", "libjpeg62-dev"),
    (D, "/usr/lib/libQtXmlPatterns.so.4",                                  "-lQtXmlPatterns", "libqt4-xmlpatterns", "libqt4-dev"),
    (D, "/usr/lib/libQtScript.so.4",                                       "-lQtScript", "libqt4-script", "libqt4-dev"),
    (D, "/usr/lib/libssl.so.0.9.8",                                        "-lssl", "libssl0.9.8", "libssl-dev"),
    (D, "/usr/lib/libaegis_crypto.so.1",                                   "-laegis_crypto", "libaegis-crypto1", "libaegis-crypto-dev"),
    (D, "/usr/lib/libQtDeclarative.so.4",                                  "-lQtDeclarative", "libqt4-declarative", "libqt4-dev"),
    (D, "/usr/lib/libxml2.so.2",                                           "-lxml2", "libxml2", "libxml2-dev"),
    (D, "/lib/libcal.so.1",                                                "-lcal", "libcal1", "libcal-dev"),
    (D, "/usr/lib/libcontentaction.so.0",                                  "-lcontentaction", "libcontentaction0", "libcontentaction-dev"),
    (D, "/usr/lib/libcrypto.so.0.9.8",                                     "-lcrypto", "libssl0.9.8", "libssl-dev"),
    (D, "/usr/lib/libbb5.so.0",                                            "-lbb5", "libbb5-0", "libbb5-dev"),
    (D, "/lib/libresolv-2.10.1.so",                                        "-lresolv-2.10.1", "libc6", "libc6"),
    (D, "/usr/lib/libgio-2.0.so.0",                                        "-lgio-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/usr/lib/libQtSparql.so.0",                                       "-lQtSparql", "libqtsparql0", "libqtsparql-dev"),
    (D, "/usr/lib/libsqlite3.so.0",                                        "-lsqlite3", "libsqlite3-0", "libsqlite3-dev"),
    (D, "/usr/lib/libQtSql.so.4",                                          "-lQtSql", "libqt4-sql", "libqt4-dev"),
    (D, "/usr/lib/libmeegotouchsettings.so.0",                             "-lmeegotouchsettings", "libmeegotouchsettings0", "libmeegotouch-dev"),
    (D, "/usr/lib/libmeegotouchextensions.so.0",                           "-lmeegotouchextensions", "libmeegotouchextensions0", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libminputcontext.so",           "-lminputcontext", "meego-im-context", "meego-im-context"),
    (D, "/usr/lib/qt4/plugins/inputmethods/libqimsw-multi.so",             "-lqimsw-multi", "libqtgui4", "libqtgui4"),
    (D, "/lib/libudev.so.0",                                               "-ludev", "libudev0", "libudev-dev"),
    (D, "/usr/lib/libmeegotouchviews.so.0",                                "-lmeegotouchviews", "libmeegotouchviews0", "libmeegotouch-dev"),
    (D, "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "/usr/lib/qt4/plugins/graphicssystems/libqmeegographicssystem.so", "libqt4-meegographicssystem", "libqt4-meegographicssystem"),
    (D, "/usr/lib/libXrandr.so.2",                                         "-lXrandr", "libxrandr2", "libxrandr-dev"),
    (D, "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so",            "/usr/lib/contextkit/subscriber-plugins/kbslider-1.so", "context-keyboard", "context-keyboard"),
    (D, "/usr/lib/libXcomposite.so.1",                                     "-lXcomposite", "libxcomposite1", "libxcomposite-dev"),
    (D, "/usr/lib/libGLESv2_r125.so",                                      "-lGLESv2_r125", "libgles2-sgx-img", "libgles2-sgx-img"),
    (D, "/usr/lib/libgconf-2.so.4",                                        "-lgconf-2", "libgconf2-6", "libgconf2-dev"),
    (D, "/usr/lib/libQtCore.so.4",                                         "-lQtCore", "libqtcore4", "libqt4-dev"),
    (D, "/usr/lib/libdbus-glib-1.so.2",                                    "-ldbus-glib-1", "libdbus-glib-1-2", "libdbus-glib-1-dev"),
    (D, "/usr/lib/libstdc++.so.6",                                         "-lstdc++", "libstdc++6", "libstdc++6-4.4-dev"),
    (D, "/usr/lib/libXi.so.6",                                             "-lXi", "libxi6", "libxi-dev"),
    (D, "/lib/libpcre.so.3",                                               "-lpcre", "libpcre3", "libpcre3-dev"),
    (D, "/usr/lib/libQtSvg.so.4",                                          "-lQtSvg", "libqt4-svg", "libqt4-dev"),
    (D, "/usr/lib/libicui18n.so.44",                                       "-licui18n", "libicu44", "libicu-dev"),
    (D, "/usr/lib/libIMGegl_r125.so",                                      "-lIMGegl_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    (D, "/usr/lib/libdbus-1.so.3",                                         "-ldbus-1", "libdbus-1-3", "libdbus-1-dev"),
    (D, "/usr/lib/libmeegotouchcore.so.0",                                 "-lmeegotouchcore", "libmeegotouchcore0", "libmeegotouch-dev"),
    (D, "/usr/lib/libX11.so.6",                                            "-lX11", "libx11-6", "libx11-dev"),
    (D, "/usr/lib/libexpat.so.1",                                          "-lexpat", "libexpat1", "libexpat1-dev"),
    (D, "/usr/lib/libfontconfig.so.1",                                     "-lfontconfig", "libfontconfig1", "libfontconfig1-dev"),
    (D, "/usr/lib/libSM.so.6",                                             "-lSM", "libsm6", "libsm-dev"),
    (D, "/usr/lib/libICE.so.6",                                            "-lICE", "libice6", "libice-dev"),
    (D, "/usr/lib/libEGL_r125.so",                                         "-lEGL_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    (D, "/usr/lib/libXdamage.so.1",                                        "-lXdamage", "libxdamage1", "libxdamage-dev"),
    (D, "/usr/lib/libcontextsubscriber.so.0",                              "-lcontextsubscriber", "libcontextsubscriber0", "libcontextsubscriber-dev"),
    (D, "/lib/libdl-2.10.1.so",                                            "-ldl-2.10.1", "libc6", "libc6"),
    (D, "/usr/lib/libXext.so.6",                                           "-lXext", "libxext6", "libxext-dev"),
    (D, "/lib/libpthread-2.10.1.so",                                       "-lpthread-2.10.1", "libc6", "libc6"),
    (D, "/usr/lib/libXau.so.6",                                            "-lXau", "libxau6", "libxau-dev"),
    (D, "/usr/lib/libgobject-2.0.so.0",                                    "-lgobject-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/usr/lib/libcdb.so.1",                                            "-lcdb", "libcdb1", "libcdb-dev"),
    (D, "/usr/lib/libQtNetwork.so.4",                                      "-lQtNetwork", "libqt4-network", "libqt4-dev"),
    (D, "/usr/lib/gconv/UTF-16.so",                                        "/usr/lib/gconv/UTF-16.so", "libc6", "libc6"),
    (D, "/usr/lib/libicudata.so.44",                                       "-licudata", "libicu44", "libicu-dev"),
    (D, "/usr/lib/libQtMeeGoGraphicsSystemHelper.so.4",                    "-lQtMeeGoGraphicsSystemHelper", "libqt4-meegographicssystemhelper", "libqt4-meegographicssystemhelper-dev"),
    (D, "/usr/lib/libQtGui.so.4",                                          "-lQtGui", "libqtgui4", "libqt4-dev"),
    (D, "/usr/lib/libQtOpenGL.so.4",                                       "-lQtOpenGL", "libqt4-opengl", "libqt4-dev"),
    (D, "/usr/lib/libQtXml.so.4",                                          "-lQtXml", "libqt4-xml", "libqt4-dev"),
    (D, "/usr/lib/libQtDBus.so.4",                                         "-lQtDBus", "libqt4-dbus", "libqt4-dev"),
    (D, "/usr/lib/libXfixes.so.3",                                         "-lXfixes", "libxfixes3", "libxfixes-dev"),
    (D, "/usr/lib/libz.so.1",                                              "-lz", "zlib1g", "zlib1g-dev"),
    (D, "/usr/lib/libxcb.so.1",                                            "-lxcb", "libxcb1", "libxcb1-dev"),
    (D, "/lib/libm-2.10.1.so",                                             "-lm-2.10.1", "libc6", "libc6"),
    (D, "/usr/lib/libgmodule-2.0.so.0",                                    "-lgmodule-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/usr/lib/libsrv_um_r125.so",                                      "-lsrv_um_r125", "opengles-sgx-img-common", "opengles-sgx-img-common"),
    (D, "/usr/lib/libpng12.so.0",                                          "-lpng12", "libpng12-0", "libpng12-dev"),
    (D, "/usr/lib/libfreetype.so.6",                                       "-lfreetype", "libfreetype6", "libfreetype6-dev"),
    (D, "/usr/lib/libgthread-2.0.so.0",                                    "-lgthread-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/lib/libglib-2.0.so.0",                                           "-lglib-2.0", "libglib2.0-0", "libglib2.0-dev"),
    (D, "/usr/lib/libXrender.so.1",                                        "-lXrender", "libxrender1", "libxrender-dev"),
    (D, "/usr/lib/libicuuc.so.44",                                         "-licuuc", "libicu44", "libicu-dev"),
    (D, "/lib/librt-2.10.1.so",                                            "-lrt-2.10.1", "libc6", "libc6"),
]

import sys
import fileinput
import re
import string
import subprocess
from collections import defaultdict

def find_roots(initial_libs):
    pulled_in_by = defaultdict(list)
    dep_reg = re.compile(r"^.*=>\s*(\S*)\s.*$")
    for lib in initial_libs:
        if pulled_in_by[lib] == []:

            # potential root, mark all its dependant libraries
            cout, cerr = subprocess.Popen(["ldd", lib], stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
            for line in cout.split("\n"):
                m = dep_reg.match(line)
                if m:
                    l = m.group(1)
                    if l != "":
                        pulled_in_by[l].append(lib)

    # construct list of dependency tree roots
    roots = []
    for lib in initial_libs:
        if not pulled_in_by[lib]: 
            roots.append(lib)
    return roots

def build_deps_from_debian_control(control_file):
    res = set()
    dep_reg = re.compile(r"^Build-Depends:(.*)$")
    for line in fileinput.input(control_file):
        m = dep_reg.match(line)
        if m:
            for dep in m.group(1).split(","):
                res.add(dep.strip())
            fileinput.close()
            return res

def applauncherd_launcher_deps_from_debian_control(control_file):
    res = set()
    package_reg = re.compile(r"^Package: applauncherd-launcher$")
    dep_reg = re.compile(r"^Depends:(.*)$")

    line_match = False
    for line in fileinput.input(control_file):
        if not line_match:
            # skip until applauncherd-launcher package found
            line_match = package_reg.match(line)
        else:
            # then find the depends line
            m = dep_reg.match(line)
            if m:
                for dep in m.group(1).split(","):
                    res.add(dep.strip())
                fileinput.close()
                return res

if __name__ == "__main__":
    initial_libs = []
    linker_flags = {}
    bin_package = {}
    dev_package = {}
    status = {}

    # collect information from the library list
    for st, lib, lf, bp, dp in libraries:
        status[lib] = st
        initial_libs.append(lib)
        linker_flags[lib] = lf
        bin_package[lib] = bp
        dev_package[lib] = dp

    # roots of the library dependency tree
    roots = find_roots(initial_libs)

    if len(sys.argv) == 2 and sys.argv[1] == "--preload-h-libraries":
        f = open("preload-h-libraries.h", "w")
        f.write("// List of libraries produced by library-helper.py. DO NOT EDIT\n")
        for lib in roots:
            if status[lib] == D:
                f.write('"%s",\n' % lib)
        f.close()
    elif len(sys.argv) == 2 and sys.argv[1] == "--linker-flags":
        # produce minimized linker line
        f = open("additional-linked-libraries.ld", "w")
        for lib in roots:
            if status[lib] == L:
                f.write('%s ' % linker_flags[lib])
        f.close()
    elif len(sys.argv) == 3 and sys.argv[1] == "--check-dependencies":
        all_dev_packages = set()
        runtime_packages = set()
        linker_lib_packages = set()
        for status, lib, linkerflags, binpackage, devpackage in libraries:
            all_dev_packages.add(devpackage)
            if status == L and lib in roots:
                linker_lib_packages.add(devpackage)
                runtime_packages.add(binpackage)
        build_deps = build_deps_from_debian_control(sys.argv[2])
        runtime_deps = applauncherd_launcher_deps_from_debian_control(sys.argv[2])

        if not build_deps.issuperset(linker_lib_packages):
            print "Missing build dependencies: %s" % string.join(linker_lib_packages.difference(build_deps), ", ")
        if build_deps.difference(real_build_deps.union(linker_lib_packages)):
            print "Redundant build dependencies: %s" % string.join(build_deps.difference(real_build_deps.union(linker_lib_packages)), ", ")
        if not build_deps.issuperset(real_build_deps):
            print "Missing real build dependencies: %s" % string.join(real_build_deps.difference(build_deps), ", ")

        if not runtime_deps.issuperset(runtime_packages):
            print "Missing runtime dependencies: %s" % string.join(runtime_packages.difference(runtime_deps), ", ")
        if runtime_deps.difference(real_runtime_deps.union(runtime_packages)):
            print "Redundant runtime dependencies: %s" % string.join(runtime_deps.difference(real_runtime_deps.union(runtime_packages)), ", ")
        if not runtime_deps.issuperset(real_runtime_deps):
            print "Missing real runtime dependencies: %s" % string.join(real_runtime_deps.difference(runtime_deps), ", ")

    else:
        print """Usage: library-helper.py option
           Options: 
              --preload-h-libraries 
                   Produce a list of libraries in preload-h-libraries.h
              --linker-flags
                   Produdce a linker line fragment in additional-linked-libraries.ld
              --check-dependencies control-file
                   Given the debian control file, check if there are missing packages
                   or redundant packages in Build-Depends or applauncherd-launcher Depends.
"""

        sys.exit(1)

