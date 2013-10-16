#!/bin/bash

# script to download, compile and install the crosstool utility 'ct-ng'

VERSION=1.17.0
INSTALLDIR=/opt/crosstool-ng

wget -c http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-$VERSION.tar.bz2
wget -c http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-$VERSION.tar.bz2.sha512
sha512sum -c crosstool-ng-$VERSION.tar.bz2.sha512
tar xjf crosstool-ng-$VERSION.tar.bz2
cd crosstool-ng-$VERSION
./configure --prefix $INSTALLDIR
make
make install
cd $CWD


