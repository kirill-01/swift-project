#!/bin/bash

echo "Building packages..."
rm -rf *.deb > /dev/null
find . -type d -name 'bin' -exec echo rm -rf {}/* \; > /dev/null

str=$(cat VERSION)
increment_version() {
 local v=$1
 if [ -z $2 ]; then
    local rgx='^((?:[0-9]+\.)*)([0-9]+)($)'
 else
    local rgx='^((?:[0-9]+\.){'$(($2-1))'})([0-9]+)(\.|$)'
    for (( p=`grep -o "\."<<<".$v"|wc -l`; p<$2; p++)); do
       v+=.0; done; fi
 val=`echo -e "$v" | perl -pe 's/^.*'$rgx'.*$/$2/'`
 echo "$v" | perl -pe s/$rgx.*$'/${1}'`printf %0${#val}s $(($val+1))`/
}

newver=$(increment_version $str)

find . -type f -name 'VERSION' -exec sed -i "s/${str}/${newver}/g" {} \;
find . -type f -name 'control' -exec sed -i "s/${str}/${newver}/g" {} \;
find . -type f -name 'main.cpp' -exec sed -i "s/${str}/${newver}/g" {} \;
find . -type f -name '*.ini' -exec sed -i "s/${str}/${newver}/g" {} \;
find . -type f -name '*.pro' -exec sed -i "s/${str}/${newver}/g" {} \;

EXCHANGES=("$(cat exchanges_list.env)")
echo "Exchanges: "${EXCHANGES}

for item in ${EXCHANGES[*]}
do
    cp -f wamp_exchange.dist swift-${item}/deb/opt/swift-bot/modules/${item}/wamp.json
    sed -i "s/exchangename/${item}/g" swift-${item}/deb/opt/swift-bot/modules/${item}/wamp.json
done

MODULES=("$(cat modules_list.env)")

echo "Modules: "${MODULES}

SRCDIR=${PWD}

build_module() {
	local modulename=$1
	cd ${modulename}
	cat ../postinst.dist > deb/DEBIAN/postinst
	sed -i "s/MODULENAME/${modulename}/" deb/DEBIAN/postinst
	cat ../preinst.dist > deb/DEBIAN/preinst
	chmod 0775 deb/DEBIAN/postinst && chmod +x deb/DEBIAN/postinst
	sed -i "s/MODULENAME/${modulename}/" deb/DEBIAN/preinst
	chmod 0775 deb/DEBIAN/preinst && chmod +x deb/DEBIAN/preinst
	cat ../_module_build.dist > build_module.sh
	./build_module.sh ${2} ${3} ${4}
	(($? != 0)) && { printf '%s\n' "Command exited with non-zero"; exit 1; }
	cd ${SRCDIR}
}


# build_module qmsgpack

cd ${SRCDIR}
build_module swift-corelib ${1} ${2} ${3}
cd ${SRCDIR}

sudo dpkg -i swift-corelib*.deb

for item in ${MODULES[*]}
do
    build_module ${item} ${1} ${2} ${3}
    cd ${SRCDIR}
done

echo "All packages builded OK =)"
rm -rf packages
mkdir -p packages
mv ${SRCDIR}/*.deb packages/
# find . -type d -name 'bin' -exec echo rm -rf {}/* \;
