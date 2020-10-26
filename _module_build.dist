#!/bin/bash
MODULE_NAME=$(basename $PWD)
qmake ${1} ${2} .
make -s -j4
(($? != 0)) && { printf '%s\n' "Command exited with non-zero"; exit 1; }
[ -f $PWD/${MODULE_NAME} ] && { 
    [ -d deb/opt/swift-bot/bin ] && cp $PWD/${MODULE_NAME} deb/opt/swift-bot/bin/${MODULE_NAME}
}
[ -f $PWD/libswift-corelib.so ] && cp -f $PWD/libswift-corelib.so* deb/usr/lib/
[ ${MODULE_NAME} = "swift-server" ] && cp $PWD/${MODULE_NAME} deb/usr/bin/${MODULE_NAME}
[ ${MODULE_NAME} = "swift-console" ] && cp $PWD/${MODULE_NAME} deb/usr/bin/${MODULE_NAME}
fakeroot dpkg-deb --build deb
(($? != 0)) && { printf '%s\n' "Command exited with non-zero"; exit 1; }
mv deb.deb ../${MODULE_NAME}.deb

[ -f "deb/opt/swift-bot/bin/${MODULE_NAME}" ] && rm -rf deb/opt/swift-bot/bin/${MODULE_NAME}
[ -d "deb/opt/swift-bot/lib" ] && rm -rf deb/opt/swift-bot/lib/*

[ -z $3 ] || make -s -j4 distclean
