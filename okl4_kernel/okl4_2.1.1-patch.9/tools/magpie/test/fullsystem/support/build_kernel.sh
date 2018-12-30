#!/bin/sh

ORIGDIR=`pwd`
BUILDDIR=$ORIGDIR/build

# Set up the build environment
mkdir $BUILDDIR

# Build the kernel
cd pistachio/kernel
make BUILDDIR=$BUILDDIR/kernel
cd $BUILDDIR/kernel
cat >Makeconf.local <<EOF
ARCH=ia32
CPU=p4
PLATFORM=pc99
EOF
echo "BUILDDIR = " $BUILDDIR/kernel >>Makeconf.local
echo "SRCDIR = " $ORIGDIR/pistachio/kernel >>Makeconf.local
tar -xvf $ORIGDIR/support/kernel_config.tar
make

cd $ORIGDIR
