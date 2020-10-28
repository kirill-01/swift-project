QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_GLOBAL_LOGGER SWIFT_USE_WAMP_CLIENT SWIFT_USE_MYSQL_DB SWIFT_MODULE_INITIAL_SQL SWIFT_SINGLE_INSTANCE SWIFT_ASYNC_WORKER SWIFT_LISTEN_ORDERBOOKS

SOURCES += main.cpp \
    blockcypherapi.cpp

target.path = /opt/swift-bot/bin
include(../shared.pri)

HEADERS +=     module_definitions.h     \
    blockcypherapi.h

DISTFILES += \
    deb/opt/swift-bot/modules/blockcypher/blockcypher.dist \
    deb/opt/swift-bot/modules/blockcypher/wamp.json

