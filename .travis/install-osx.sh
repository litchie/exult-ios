#!/bin/sh

MACPORTS_URL=https://distfiles.macports.org/MacPorts
MACPORTS_PKG=MacPorts-2.3.4-10.11-ElCapitan.pkg
MACPORTS_PREFIX=/opt/local
MACPORTS_PATH=/tmp/$MACPORTS_PKG

# Download and install MacPorts
curl $MACPORTS_URL/$MACPORTS_PKG > $MACPORTS_PATH || exit 1
sudo installer -pkg $MACPORTS_PATH -target / || exit 2
export PATH=$MACPORTS_PREFIX/bin:$PATH

# Just to be sure
sudo port -v selfupdate
sudo port upgrade outdated

# Install actual dependencies
sudo port install -q automake libtool pkgconfig libvorbis libpng zlib libglade2
sudo port install -q libsdl +x11
sudo port install -q libsdl2 +x11

