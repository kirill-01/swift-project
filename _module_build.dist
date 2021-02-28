#!/bin/bash
MODULE_NAME=$(basename $PWD)
PKG_VERSION=$(cat VERSION)
PKG_NAME=${MODULE_NAME}-${PKG_VERSION}.deb
qmake ${1} ${2} .
make -s -j4
(($? != 0)) && { printf '%s\n' "Command exited with non-zero"; exit 1; }
[ -f $PWD/${MODULE_NAME} ] && { 
    [ -d deb/opt/swift-bot/bin ] && cp $PWD/${MODULE_NAME} deb/opt/swift-bot/bin/${MODULE_NAME}
}

[ -f $PWD/libswift-corelib.so ] && {
    qtchooser ${1} -print-env > /tmp/qt.env
    . /tmp/qt.env
    ldd ${PWD}/../swift-server/swift-server | grep ${QTLIBDIR} | sed "s/(.*)//g" | sed "s/.*> //g" > deb/depencies.txt
    
    [ -d deb/usr/lib/swiftbot/depends ] && rm -rf deb/usr/lib/swiftbot/depends
    mkdir -p deb/usr/lib/swiftbot/depends
    
    while read p; do
        cp ${p} deb/usr/lib/swiftbot/depends/
    done <deb/depencies.txt
    
    echo "List of depended files"
    cat deb/depencies.txt
    rm -rf /tmp/qt.env
    cp -f $PWD/libswift-corelib.so* deb/usr/lib/
    mkdir -p deb/usr/include/swiftbot
    cp -f $PWD/*.h deb/usr/include/swiftbot/
}


[ ${MODULE_NAME} = "swift-server" ] && cp $PWD/${MODULE_NAME} deb/usr/bin/${MODULE_NAME}
[ ${MODULE_NAME} = "swift-console" ] && cp $PWD/${MODULE_NAME} deb/usr/bin/${MODULE_NAME}
[ ${MODULE_NAME} = "swift-gui" ] && cp $PWD/${MODULE_NAME} deb/usr/bin/${MODULE_NAME}
[ -f ../${PKG_NAME} ] && rm -rf ../${PKG_NAME}
fakeroot dpkg-deb --build deb ../${PKG_NAME}
(($? != 0)) && { printf '%s\n' "Command exited with non-zero"; exit 1; }


[ -f "deb/opt/swift-bot/bin/${MODULE_NAME}" ] && rm -rf deb/opt/swift-bot/bin/${MODULE_NAME}
[ -d "deb/opt/swift-bot/lib" ] && rm -rf deb/opt/swift-bot/lib/*

[ -z $3 ] || make -s -j4 distclean
