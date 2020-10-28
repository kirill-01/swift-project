QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientbitfinex.cpp \
        swiftapiparserbitfinex.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientbitfinex.h \
    swiftapiparserbitfinex.h

include( ../shared.pri )
