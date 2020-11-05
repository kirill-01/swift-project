CONFIG(debug, debug|release) {
    message(DEBUG BUILD  $$TARGET)
    INCLUDEPATH += $$PWD/swift-corelib
    DEPENDPATH += $$PWD/build-corelib
    LIBS += -L$$PWD/build-corelib/  -lswift-corelib
}

CONFIG(release, debug|release) {
    message(RELEASE build $$TARGET)
    INCLUDEPATH += /usr/include/swiftbot
    DEPENDPATH += /usr/lib /usr/lib/swiftbot/depends
    LIBS += -lswift-corelib
}
