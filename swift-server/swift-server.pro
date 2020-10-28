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

include( ../shared.pri )
