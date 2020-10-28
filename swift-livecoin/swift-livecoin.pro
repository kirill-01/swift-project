QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientlivecoin.cpp \
        swiftapiparserlivecoin.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientlivecoin.h \
    swiftapiparserlivecoin.h

include(../shared.pri)
