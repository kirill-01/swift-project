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
        mainwindow.cpp


FORMS += mainwindow.ui accessruleswindow.ui \
    configeditdialog.ui

target.path = /usr/bin

include(../shared.pri)

HEADERS += \
    accessruleswindow.h \
    configeditdialog.h \
    module_definitions.h \
    mainwindow.h

RESOURCES += \
    assets.qrc


INSTALLS += target

DISTFILES += \
    deb/opt/swift-bot/modules/gui/gui.dist \
    deb/opt/swift-bot/modules/gui/wamp.json
