#!/bin/bash

set -e

# be sure that it is a desktop Qt5 implementation!
QMAKE=/usr/lib/qt5/bin/qmake

PACKAGE_NAME=hemro-ek-omnia-hmi-demonstrator-amd64
APP_NAME=HemroEkOmniaHmiDemonstrator
PURE_APP_NAME=EkxHmiDemonstrator
DEST_DIR=opt
BUILD_DIR=builddir
SUDO_CMD=""

if [ ! -e $QMAKE ]
then
	echo "fatal - can't find qmake"
	exit -1
fi

cd ..

for f in `find . -name "*.so.*"`; do
	${SUDO_CMD} rm -f $f
done

for f in `find . -name "*.so"`; do
	${SUDO_CMD} rm -f $f
done

if [ -e ${APP_NAME} ]
then
	${SUDO_CMD} rm -f ${APP_NAME}
fi

if [ -e ${APP_NAME}_d ]
then
	${SUDO_CMD} rm -f ${APP_NAME}_d
fi

for f in `find . -name "Makefile*"`; do
	${SUDO_CMD} rm $f
done

rm -rf `find . -name "builddir*"`

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
${QMAKE} -o Makefile ../EkxHmiDemonstrator.pro CONFIG+="qtquickcompiler release" -spec linux-g++
make clean > make_clean.log
make -j > make_build.log

chmod 0755 version.sh
source version.sh

cd ../DebianInstaller

PACKAGE_DIR=${PACKAGE_NAME}-${VERSION_NO}

if [ -e ${PACKAGE_DIR} ]
then
	${SUDO_CMD} rm -rf ${PACKAGE_DIR}
fi

mkdir -p ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}
mkdir -p ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}/InstallMimeTypes
mkdir -p ${PACKAGE_DIR}/usr/share/doc/${PACKAGE_NAME}
mkdir -p ${PACKAGE_DIR}/usr/share/applications
mkdir -p ${PACKAGE_DIR}/usr/share/icons
mkdir -p ${PACKAGE_DIR}/usr/lib
mkdir -p ${PACKAGE_DIR}/tmp/${PACKAGE_NAME}

cp ../InstallMimeTypes/${APP_NAME}.desktop   ${PACKAGE_DIR}/usr/share/applications
cp ../InstallMimeTypes/${APP_NAME}.png       ${PACKAGE_DIR}/usr/share/icons
cp ../${BUILD_DIR}/${PURE_APP_NAME}          ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}
cp ../InstallMimeTypes/*                     ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}/InstallMimeTypes
cp ../license.txt                            ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}/license.txt
cp ../release_notes.txt                      ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}
cp ../${BUILD_DIR}/version.sh                ${PACKAGE_DIR}/${DEST_DIR}/${APP_NAME}/version.info

cd ${PACKAGE_DIR}

mkdir DEBIAN
cp ../debian_control.txt   DEBIAN/control
cp ../debian_copyright.txt usr/share/doc/${PACKAGE_NAME}/copyright
cat ../../license.txt | sed -r 's/^(.+)/ \1/g' | sed -r 's/^\s*$/ ./g' >> usr/share/doc/${PACKAGE_NAME}/copyright
cp ../debian_preinst.sh    DEBIAN/preinst
cp ../debian_postinst.sh   DEBIAN/postinst
cp ../debian_prerm.sh      DEBIAN/prerm
cp ../debian_postrm.sh     DEBIAN/postrm

mkdir debian
dch --create --fromdirname --urgency low "see release notes in application (Closes: #12345)"
mv debian/changelog usr/share/doc/${PACKAGE_NAME}/changelog
gzip --best usr/share/doc/${PACKAGE_NAME}/changelog

cp DEBIAN/* debian
echo -e "Source: $PACKAGE_NAME\n\n" > debian/control
cat ../debian_control.txt  >> debian/control
dpkg-shlibdeps --ignore-missing-info -lusr/lib ${DEST_DIR}/${APP_NAME}/${PURE_APP_NAME}
#dpkg-shlibdeps --ignore-missing-info -lusr/lib usr/lib/*.so

DEPENDS=`cat debian/substvars | sed -r "s/shlibs:Depends=(.+)/\1/g"`
DEPENDS="$DEPENDS, libqt5quickshapes5, libqt5quickcontrols2-5, libqt5quicktemplates2-5, qml-module-qtquick-controls2, qml-module-qtqml-models2, qml-module-qtquick-shapes, qml-module-qtquick-controls, qml-module-qtgraphicaleffects"

cat ../debian_control.txt | sed -e "s/\$DEPENDS/${DEPENDS}/g" > debian/control1
cat debian/control1 | sed -e "s/\$VERSION_NO/${VERSION_NO}/g" > DEBIAN/control

rm -rf debian

chmod 0755 DEBIAN/preinst
chmod 0755 DEBIAN/postinst
chmod 0755 DEBIAN/prerm
chmod 0755 DEBIAN/postrm
#chmod 0644 `find . -name "*.so*"`

strip --strip-unneeded ${DEST_DIR}/${APP_NAME}/${PURE_APP_NAME}
#strip --strip-unneeded `find . -name "*.so*"`

cd ..
#${SUDO_CMD} chown root:root -R ${PACKAGE_DIR}

dpkg-deb --build ${PACKAGE_DIR}

${SUDO_CMD} rm -rf ${PACKAGE_DIR}

echo "Finished!"
