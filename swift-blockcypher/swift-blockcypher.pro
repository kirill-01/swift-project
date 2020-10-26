QT -= gui
QT += network websockets sql

CONFIG += c++11 console debug
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_GLOBAL_LOGGER SWIFT_USE_WAMP_CLIENT SWIFT_USE_MYSQL_DB SWIFT_MODULE_INITIAL_SQL SWIFT_SINGLE_INSTANCE SWIFT_ASYNC_WORKER SWIFT_LISTEN_ORDERBOOKS

SOURCES += main.cpp templateworker.cpp

target.path = /opt/swift-bot/bin

CONFIG(debug, debug|release) {
    message(DEBUG BUILD)
    INCLUDEPATH += $$PWD/../swift-corelib
    DEPENDPATH += $$PWD/../build-corelib
    LIBS += -L$$PWD/../build-corelib/  -lswift-corelib
}

CONFIG(release, debug|release) {
    message(RELEASE build)
    INCLUDEPATH += /usr/include/swiftbot
    DEPENDPATH += /usr/include/swiftbot
    LIBS += -lswift-corelib
}

HEADERS +=     module_definitions.h     templateworker.h

