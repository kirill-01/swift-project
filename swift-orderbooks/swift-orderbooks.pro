QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        orderbooksproxy.cpp \
        nodescontroller.cpp \
        orderbooksnode.cpp

target.path = /opt/swift-bot/bin

HEADERS += \
    orderbooksproxy.h \
    nodescontroller.h \
    orderbooksnode.h


include( ../shared.pri )

DISTFILES += \
    deb/opt/swift-bot/modules/orderbooks/orderbooks.dist \
    deb/opt/swift-bot/modules/orderbooks/wamp.json
