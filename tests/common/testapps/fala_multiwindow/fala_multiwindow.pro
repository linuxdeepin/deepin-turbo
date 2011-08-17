TEMPLATE     = app
TARGET       = fala_multi-window
target.path  = /usr/bin
OBJECTS_DIR  = ./.obj
MOC_DIR      = ./.moc
DEPENDPATH  += $$INCLUDEPATH
CONFIG      -= app_bundle
CONFIG      += meegotouch-boostable

SOURCES += main.cpp \
    mainpage.cpp \
    multiwindowcontent.cpp

HEADERS += mainpage.h \
    multiwindowcontent.h

# Install instructions
INSTALLS += target


DEFINES += HAVE_MCOMPONENTCACHE
