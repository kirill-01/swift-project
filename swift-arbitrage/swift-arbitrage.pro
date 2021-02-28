QT -= gui
QT += network websockets sql concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

include( ../shared.pri )


SOURCES += \
        main.cpp \
        marketsfilter.cpp \
        statsnotifier.cpp

target.path = /opt/swift-bot/bin



HEADERS += \
    marketsfilter.h \
    statsnotifier.h


DISTFILES += \
    deb/opt/swift-bot/modules/arbitrage/arbitrage.dist \
    deb/opt/swift-bot/modules/arbitrage/wamp.json

