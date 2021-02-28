QT -= gui
QT += network websockets sql

TEMPLATE = lib
VERSION = 1.0.493
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


CONFIG(debug, debug|release) {
    message(DEBUG BUILD $$TARGET)
    INCLUDEPATH += $$PWD/../qmsgpack/src
    DEPENDPATH += $$PWD/../qmsgpack/src
    LIBS += -L$$OUT_PWD/../qmsgpack/bin/ -lqmsgpack
}

CONFIG(release, debug|release) {
    message(RELEASE build $$TARGET)
    headers.path    = /usr/include/swiftbot
    headers.files   += $$HEADERS
    INSTALLS += headers target
    INCLUDEPATH += $$PWD/../qmsgpack/src
    DEPENDPATH += $$PWD/../qmsgpack/src
    LIBS += -L$$OUT_PWD/../qmsgpack/bin/ -lqmsgpack
}

DISTFILES += \
    ../_module_build.dist \
    ../modules_list.env \
    ../postinst.dist \
    ../preinst.dist \
    ../wamp_exchange.dist


