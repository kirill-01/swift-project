QT -= gui
QT += network websockets sql

CONFIG += c++11 console debug
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_GLOBAL_LOGGER  SWIFT_SINGLE_INSTANCE

SOURCES +=         main.cpp         templateworker.cpp \
    crossbarconfig.cpp

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

HEADERS +=     module_definitions.h     templateworker.h \
    crossbarconfig.h

