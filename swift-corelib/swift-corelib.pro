QT -= gui
QT += network websockets sql

TEMPLATE = lib
VERSION = 1.0.343
DEFINES += SWIFTCORELIB_LIBRARY

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS
SOURCES += \
    assetsstorage.cpp \
    moduleworker.cpp \
    swiftcore.cpp \
    wamp.cpp \
    wampclient.cpp \
    wampauth.cpp \
    swiftapiclient.cpp \
    swiftapiparser.cpp \
    swiftworker.cpp

HEADERS += \
    assetsstorage.h \
    moduleworker.h \
    swift-corelib_global.h \
    swiftbot.h \
    swiftcore.h \
    systemlogger.h \
    wamp.h \
    wampclient.h \
    wampauth.h \
    swiftapiclient.h \
    swiftapiparser.h \
    swiftworker.h

target.path = /usr/lib


LIBS += -lqmsgpack


CONFIG(debug, debug|release) {
    message(DEBUG BUILD $$TARGET)
}

CONFIG(release, debug|release) {
    message(RELEASE build $$TARGET)
    headers.path    = /usr/include/swiftbot
    headers.files   += $$HEADERS
    INSTALLS += headers target
}
