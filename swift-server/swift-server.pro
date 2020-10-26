QT -= gui
QT += network websockets sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        hostinfocollector.cpp \
        logger.cpp \
        main.cpp \
        modulesmanager.cpp \
        procwrapper.cpp

target.path = /usr/bin

HEADERS += \
    hostinfocollector.h \
    logger.h \
    modulesmanager.h \
    procwrapper.h

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
