QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientzb.cpp \
        swiftapiparserzb.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientzb.h \
    swiftapiparserzb.h

include(../shared.pri)
