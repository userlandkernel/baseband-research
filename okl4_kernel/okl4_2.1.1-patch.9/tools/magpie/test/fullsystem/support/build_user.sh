#!/bin/sh

ORIGDIR=`pwd`
BUILDDIR=$ORIGDIR/build

# Build the user environment
cd $BUILDDIR
mkdir user
cd user
$ORIGDIR/pistachio/user/configure --with-comport=0
make

cd $ORIGDIR

# Create the boot disk
# FIXME: Might want to create a CD image instead -- check makeboot for details
support/makeboot.ia32.sh
