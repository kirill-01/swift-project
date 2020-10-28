QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        swiftapiclientkraken.cpp \
        swiftapiparserkraken.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    swiftapiclientkraken.h \
    swiftapiparserkraken.h

include( ../shared.pri )
