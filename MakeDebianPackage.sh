#! /bin/bash
#
# Based on https://www.hackgnar.com/2016/01/simple-deb-package-creation.html

# Note: apparently, you can use
#   objdump -p build/Debug/ChristiansSteamBot | grep NEEDED
# to get an idea about dependencies

# ToDo: for now, I don't add dependencies in here...

PACKAGE_NAME=ChristianSteamBot

SOURCE_DIR=`pwd`
ARCHITECTURE=`dpkg --print-architecture`

if [ ! -e build/Release ]; then
   cmake -S . -B build/Release -D CMAKE_BUILD_TYPE=Release || exit 1
fi
cmake --build build/Release || exit 1

rm -rf /tmp/Chriatians-Steam-Bot-Package-Build
mkdir -p "/tmp/Chriatians-Steam-Bot-Package-Build/$PACKAGE_NAME" || exit 1
cd "/tmp/Chriatians-Steam-Bot-Package-Build/$PACKAGE_NAME" || exit 1

mkdir DEBIAN || exit 1
cat <<EOF >> DEBIAN/control
Package: $PACKAGE_NAME
Maintainer: Christian Stieber <stieber.chr@gmail.com>
Architecture: $ARCHITECTURE
Priority: optional
Version: 0.1
Description: Work in progress CLI client to do some stuff on your Steam accounts
Vcs-Browser: https://github.com/Christian-Stieber/Christians-Steam-Bot
Vcs-Git: https://github.com/Christian-Stieber/Christians-Steam-Bot.git -b main
EOF

mkdir -p usr/bin || exit 1
cp "$SOURCE_DIR/build/Release/ChristiansSteamBot" usr/bin/ || exit 1
chmod 755 usr/bin/ChristiansSteamBot || exit 1

cd .. || exit 1
dpkg-deb --build "$PACKAGE_NAME" "$SOURCE_DIR/$PACKAGE_NAME-$ARCHITECTURE.deb" || exit 1
echo "Package $PACKAGE_NAME-$ARCHITECTURE.deb completed"
