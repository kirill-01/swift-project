QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientbittrex.cpp \
        swiftapiparserbittrex.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientbittrex.h \
    swiftapiparserbittrex.h

include(../shared.pri)
