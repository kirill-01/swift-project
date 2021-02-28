CONFIG(debug, debug|release) {
    message(DEBUG BUILD  $$TARGET)
    INCLUDEPATH += $$PWD/swift-corelib $$PWD/qmsgpack/src
    DEPENDPATH += $$PWD/swift-corelib $$PWD/qmsgpack/src
    LIBS += -L$$OUT_PWD/../swift-corelib/ -lswift-corelib -L$$OUT_PWD/../qmsgpack/bin/ -lqmsgpack

}

CONFIG(release, debug|release) {
    message(RELEASE build $$TARGET)
    INCLUDEPATH += /usr/include/swiftbot
    DEPENDPATH += /usr/lib /usr/lib/swiftbot/depends
    LIBS += -lswift-corelib
}
