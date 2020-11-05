CONFIG(debug, debug|release) {
    message(DEBUG BUILD  $$TARGET)
    INCLUDEPATH += $$PWD/swift-corelib
    DEPENDPATH += $$PWD/../build-swift-project/swift-corelib
    LIBS += -L$$PWD/../build-swift-project/swift-corelib/  -lswift-corelib
}

CONFIG(release, debug|release) {
    message(RELEASE build $$TARGET)
    INCLUDEPATH += /usr/include/swiftbot
    DEPENDPATH += /usr/lib /usr/lib/swiftbot/depends
    LIBS += -lswift-corelib
}
