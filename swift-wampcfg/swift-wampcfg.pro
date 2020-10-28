QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_GLOBAL_LOGGER SWIFT_SINGLE_INSTANCE

SOURCES +=         main.cpp crossbarconfig.cpp

target.path = /opt/swift-bot/bin

include( ../shared.pri )

HEADERS +=     module_definitions.h  crossbarconfig.h

