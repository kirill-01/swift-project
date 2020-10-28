QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        consolereader.cpp \
        main.cpp \
        resultwaiter.cpp

target.path = /usr/bin

INSTALLS += target

include(../shared.pri)
HEADERS += \
    consolereader.h \
    resultwaiter.h

DISTFILES += \
    deb/opt/swift-bot/modules/console/wamp.json
