QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_USE_WAMP_CLIENT

SOURCES += \
        main.cpp \
        orderskeeper.cpp

target.path = /opt/swift-bot/bin

include(../shared.pri)

HEADERS += \
    orderskeeper.h

DISTFILES += \
    deb/opt/swift-bot/modules/orders/orders.dist \
    deb/opt/swift-bot/modules/orders/wamp.json

