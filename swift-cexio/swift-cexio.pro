QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientcexio.cpp \
        swiftapiparsercexio.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientcexio.h \
    swiftapiparsercexio.h

include(../shared.pri)
