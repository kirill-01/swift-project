QT += gui network websockets sql widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS \
        SWIFT_USE_WAMP_CLIENT \
        SWIFT_GLOBAL_LOGGER \
        SWIFT_USE_MYSQL_DB \
        SWIFT_SINGLE_INSTANCE


SOURCES += \
        accessruleswindow.cpp \
        configeditdialog.cpp \
        main.cpp \
        templateworker.cpp \
        mainwindow.cpp


FORMS += mainwindow.ui accessruleswindow.ui \
    configeditdialog.ui

target.path = /usr/bin

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

HEADERS += \
    accessruleswindow.h \
    configeditdialog.h \
    module_definitions.h \
    templateworker.h \
    mainwindow.h

RESOURCES += \
    assets.qrc


INSTALLS += target
