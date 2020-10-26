QT -= gui
QT += network websockets sql concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        marketsfilter.cpp \
        statsnotifier.cpp

target.path = /opt/swift-bot/bin



HEADERS += \
    marketsfilter.h \
    statsnotifier.h


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
