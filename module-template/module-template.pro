QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS \
        SWIFT_USE_WAMP_CLIENT \
        SWIFT_GLOBAL_LOGGER \
        SWIFT_USE_MYSQL_DB \
        SWIFT_MODULE_INITIAL_SQL \
        SWIFT_SINGLE_INSTANCE \
        SWIFT_ASYNC_WORKER \
        SWIFT_LISTEN_ORDERBOOKS \
        SWIFT_LISTEN_BALANCE \
        SWIFT_LISTEN_ORDERS

SOURCES += \
        main.cpp \
        templateworker.cpp

HEADERS += \
    module_definitions.h \
    templateworker.h

target.path = /opt/swift-bot/bin

include( ../shared.pri )


