QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS \
           SWIFT_GLOBAL_LOGGER \
           SWIFT_USE_MYSQL_DB \
           SWIFT_MODULE_INITIAL_SQL \
           SWIFT_SINGLE_INSTANCE \
           SWIFT_USE_WAMP_CLIENT

SOURCES +=         main.cpp         templateworker.cpp \
    node.cpp \
    profitmanager.cpp

target.path = /opt/swift-bot/bin

include(../shared.pri)

HEADERS +=     module_definitions.h     templateworker.h \
    node.h \
    profitmanager.h \
    swift_ext.h

DISTFILES += \
    deb/opt/swift-bot/modules/pamm/wamp.json \
    deb/opt/swift-bot/modules/pamm/pamm.dist
