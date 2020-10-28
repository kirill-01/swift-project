QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclienthuobi.cpp \
        swiftapiparserhuobi.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclienthuobi.h \
    swiftapiparserhuobi.h

include( ../shared.pri )
