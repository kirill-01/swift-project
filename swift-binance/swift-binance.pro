QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientbinance.cpp \
        swiftapiparserbinance.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientbinance.h \
    swiftapiparserbinance.h

include( ../shared.pri )
