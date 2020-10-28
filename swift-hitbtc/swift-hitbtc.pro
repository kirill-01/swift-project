QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclienthitbtc.cpp \
        swiftapiparserhitbtc.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclienthitbtc.h \
    swiftapiparserhitbtc.h

include( ../shared.pri )
