#!/bin/bash
source ./module_env
SOURCE_DIR=${PWD}
DEST_DIR="${PWD}/../swift-${MODULE_NAME}"
echo "Initializing  new module, named : ${MODULE_NAME}..."

RES=$(find ../ -type d -name swift-${MODULE_NAME})

if [ ! -z ${RES} ]; then
   rm -rf ../swift-${MODULE_NAME}
fi 

# Copy directory
echo "Creating module files ${DEST_DIR}"
cp -rf "${SOURCE_DIR}" "${DEST_DIR}"


FULLPATH=$(realpath ${DEST_DIR})

echo "New module location: ${FULLPATH}"
cd "${FULLPATH}"

cat ${FULLPATH}/module_definitions.h
fromstr="swift-template"
tostr="swift-${MODULE_NAME}"
find ${FULLPATH} -type f -name 'module_definitions.h' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'consorol' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'postinst' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'preinst' -exec sed -i "s/${fromstr}/${tostr}/" {} \;

mv -f ${FULLPATH}/deb/opt/swift-bot/modules/TEMPLATE ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}
mv -f ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}/*.dist ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}/${MODULE_NAME}.dist

sed -i "s/name=template/name=${MODULE_NAME}/" ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}/${MODULE_NAME}.dist
sed -i "s/description=Simple module example/description=${MODULE_DESCRIPTION}/" ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}/${MODULE_NAME}.dist
sed -i "s/swift-template/swift-${MODULE_NAME}/" ${FULLPATH}/deb/opt/swift-bot/modules/${MODULE_NAME}/${MODULE_NAME}.dist





find ${FULLPATH} -type f -name 'control' -exec sed -i "s/TEMPLATEDESCRIPTION/${MODULE_DESCRIPTION}/" {} \;


fromstr="swift-template"
tostr="swift-${MODULE_NAME}"
find ${FULLPATH} -type f -name 'control' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'postinst' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'preinst' -exec sed -i "s/${fromstr}/${tostr}/" {} \;
find ${FULLPATH} -type f -name 'module_definitions.h' -exec sed -i "s/${fromstr}/${tostr}/" {} \;





fromstr="SwiftBot simple module template"
tostr=${MODULE_DESCRIPTION}
find ${FULLPATH} -type f -name 'module_definitions.h' -exec sed -i "s/${fromstr}/${tostr}/" {} \;


fromstr='"template"'
tostr=\"${MODULE_NAME}\"
find ${FULLPATH} -type f -name 'module_definitions.h' -exec sed -i "s/${fromstr}/${tostr}/" {} \;

rm -rf ${FULLPATH}/initmodule.sh
rm -rf ${FULLPATH}/module.env
mv -f ${FULLPATH}/module-template.pro ${FULLPATH}/swift-${MODULE_NAME}.pro
rm -rf ${FULLPATH}/*.user 
rm -rf ${FULLPATH}/Makefile
mv ${FULLPATH}/deb/opt/swift-bot/sql/99-template.sql ${FULLPATH}/deb/opt/swift-bot/sql/99-${MODULE_NAME}.sql

cat <<EOF > ${FULLPATH}/build_module.sh
#!/bin/bash
MODULE_NAME=\$(basename \$PWD)
qmake -qt=qt514 .
make -s -j4
cp \${MODULE_NAME} deb/opt/swift-bot/bin/\${MODULE_NAME}
fakeroot dpkg-deb --build deb
mv deb.deb ../\${MODULE_NAME}.deb
rm -rf deb/opr/swift-bot/bin/\${MODULE_NAME}
[ -z \$1 ] || make -s -j4 distclean
EOF

chmod +x ${FULLPATH}/build_module.sh

cat <<EOF > ${FULLPATH}/swift-${MODULE_NAME}.pro
QT -= gui
QT += network websockets sql

CONFIG += c++11 console debug
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS SWIFT_USE_WAMP_CLIENT SWIFT_GLOBAL_LOGGER SWIFT_USE_MYSQL_DB SWIFT_MODULE_INITIAL_SQL SWIFT_SINGLE_INSTANCE SWIFT_ASYNC_WORKER SWIFT_LISTEN_ORDERBOOKS SWIFT_LISTEN_BALANCE SWIFT_LISTEN_ORDERS

SOURCES += \
        main.cpp \
        templateworker.cpp

target.path = /opt/swift-bot/bin

CONFIG(debug, debug|release) {
    message(DEBUG BUILD)
    INCLUDEPATH += \$\$PWD/../swift-corelib
    DEPENDPATH += \$\$PWD/../build-corelib
    LIBS += -L\$\$PWD/../build-corelib/  -lswift-corelib
}

CONFIG(release, debug|release) {
    message(RELEASE build)
    INCLUDEPATH += /usr/include/swiftbot
    DEPENDPATH += /usr/include/swiftbot
    LIBS += -lswift-corelib
}

HEADERS += \
    module_definitions.h \
    templateworker.h

EOF


#sed -i "s/${fromstr}/${tostr}/" ${FULLPATH}/swift-${MODULE_NAME}.pro

#export OPTIONS_SQL=TRUE
[ ${OPTIONS_WAMP} = "TRUE" ] || sed -i "s/SWIFT_USE_WAMP_CLIENT//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_SQL} = "TRUE" ] || sed -i "s/SWIFT_USE_MYSQL_DB//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_INITIAL_SQL} = "TRUE" ] || sed -i "s/SWIFT_MODULE_INITIAL_SQL//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_LOGGER} = "TRUE" ] || sed -i "s/SWIFT_GLOBAL_LOGGER//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_SINGLETON} = "TRUE" ] || sed -i "s/SWIFT_SINGLE_INSTANCE//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_ASYNC_WORKER} = "TRUE" ] || sed -i "s/SWIFT_ASYNC_WORKER//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_LISTEN_ORDERBOOKS} = "TRUE" ] || sed -i "s/SWIFT_USE_WAMP_CLIENT//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_LISTEN_BALANCE} = "TRUE" ] || sed -i "s/SWIFT_LISTEN_BALANCE//" ${FULLPATH}/swift-${MODULE_NAME}.pro
[ ${OPTIONS_LISTEN_ORDERS} = "TRUE" ] || sed -i "s/SWIFT_LISTEN_ORDERS//" ${FULLPATH}/swift-${MODULE_NAME}.pro

#export OPTIONS_WAMP=TRUE
#export OPTIONS_SQL=TRUE
#export OPTIONS_INITIAL_SQL=TRUE
#export OPTIONS_LOGGER=TRUE
#export OPTIONS_SINGLETON=TRUE
#export OPTIONS_AUTHOR="Kirill Kuznetsov"
#export OPTIONS_SINGLE_INSTANCE=TRUE
#export OPTIONS_ASYNC_WORKER=TRUE
#export OPTIONS_LISTEN_ORDERBOOKS=TRUE
#export OPTIONS_LISTEN_BALANCE=TRUE
#export OPTIONS_LISTEN_ORDERS=TRUE
# Update config options
# e.g. .pro DEFINES LIST

#/ Create DEB definitions and filed

# Rename files, versions, params etc..



