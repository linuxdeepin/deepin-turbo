TEMPLATE = lib
TARGET = creds_test
DEPENDPATH += .
INCLUDEPATH += .

SOURCES += creds_test.cpp

CONFIG += meegotouch plugin

#QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
#QMAKE_LFLAGS += -pie -rdynamic

LIBS += -lcreds
