QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        balanceskeeper.cpp \
        main.cpp

target.path = /opt/swift-bot/bin

include( ../shared.pri )

HEADERS += \
    balanceskeeper.h

DISTFILES += \
    deb/opt/swift-bot/modules/balance/balance.dist \
    deb/opt/swift-bot/modules/balance/wamp.json
