QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        telegramapi.cpp

target.path = /opt/swift-bot/bin

include(../shared.pri)

HEADERS += \
    telegramapi.h

DISTFILES += \
    deb/opt/swift-bot/modules/telegram/telegram.dist \
    deb/opt/swift-bot/modules/telegram/wamp.json


