
OKL4 Release 2.1
================

Date: 15 April 2008

Table of Contents
-----------------

1. Introduction

2. Supported Target Platforms and Architectures

    a. ARMv5 Architectures

    b. ARMv6 Architectures

    c. IA32 Architectures

    d. Hardware Simulation

3. Prerequisites

    a. Toolchains

    b. Simulators

    c. Other Requirements

4. Building OKL4

    a. Before you begin

    b. L4 Test

    c. OKL4

    d. OK Linux

    e. How to 'make clean'

5. Getting Around the Source

6. Getting Help



Introduction
------------

OKL4 is built using cutting-edge microkernel technology, which provides Trustworthy Virtualization and Secure Decomposition capabilities for embedded systems. OKL4 supports system virtualization, secure execution of components in an execution environment with a minimal trusted computing base, and fault isolation within complex systems using lightweight protected components.



Supported Target Platforms and Architectures
--------------------------------------------

This release supports the following development platforms:

### ARMv5 Architecture

 * Gumstix - a pxa250 SoC with an ARMv5 based XScale CPU core
 * GTA01 - a s3c2410 SoC with an ARMv5 based ARM920T CPU core (used by the FIC Neo1973 'openmoko' phone)

### ARMv6 Architecture

 * KZM - a i.MX31 SoC with an ARMv6 based ARM1136 CPU core

### IA32 Architecture

 * PC99 - the standard IA32 platform

### Hardware Simulation

In addition to the hardware platforms listed above, software simulation of the Gumstix, GTA and PC99 platforms are also supported.



Prerequisites
-------------

### Toolchains

The toolchain for building on ARM-based targets can be downloaded from [here][1].

[1]: http://www.ertos.nicta.com.au/downloads/tools/arm-linux-3.4.4.tar.gz

The toolchain for building on IA32-based targets can be downloaded from [here][2].  Please also note the section "Compiling for IA32 Targets" below.

[2]: http://www.ertos.nicta.com.au/downloads/i686-gcc-3.3.4-glibc-2.3.3-2006-06-02.tar.gz

The IA32 NPTL toolchain for building OK Linux on IA32 targets can be found from [here][3].

[3]: http://wiki.ok-labs.com/downloads/release-2.1/i686-unknown-linux-gnu.tar.bz2

### Simulators

The Skyeye simulator is used to simulate Gumstix hardware.  It can be obtained [here][3].

[3]: http://www.ertos.nicta.com.au/downloads/tools/skyeye-kenge-1.2.1n.tar.gz

The QEMU simulator is used to simulate GTA (Neo1973 'openmoko phone')  and PC99 hardware.

Information about the OpenMoko QEMU can be found [here][4].

[4]: http://wiki.openmoko.org/wiki/Openmoko_under_QEMU

Information about the PC99 QEMU can be found [here][5] (version 0.8.2 is supported, although later versions may still work).

[5]: http://fabrice.bellard.free.fr/qemu/

### Other Requirements

Python 2.4.4 is required and supported, although other 2.4-based versions may still work.



Compiling OKL4
--------------

### Before you begin

Please ensure that all the prerequisite tools and simulators are in the shell path.  In particular, the default compiler `gcc` points to the one distributed with the binary.

In the build commands listed below, adding the word `simulate` at the end will automatically invoke the simulator, if the simulator is available.

The build system is invoked by running:

    $ ./tools/build.py

with appropriate arguments.  There are two required arguments, `project` and `machine`.

The `machine` argument specifies the target platform, and must be one of the following:

* `gumstix`
* `gta01`
* `kzm_arm11`
* `ia32_pc99`

### L4 Test Suite

The L4 Test Suite `l4test` performs a number of unit tests for kernel functionality.

    $ ./tools/build.py project=l4test machine=<target>

### OKL4

Specifying `project` as `iguana` will build the complete OKL4 software stack.  An additional argument `example` can be provided to specify a program to be run on OKL4.  The value should correspond to a sub-directory within `iguana/example`.

For example,

    $ ./tools/build.py project=iguana machine=<target> example=dining_philosophers

will invoke the Dining Philosophers example application.

### OKL4 with POSIX Support

OKL4 programs can be compiled with a subset of PSE51 support.  An additional argument `example_env` must be specified with `posix` as its value.

For example,

    $ ./tools/build.py project=iguana machine=<target> example=posix example_env=posix

### OKL4 Test Suite

The OKL4 Test Suite performs a number of unit tests on OKL4 functionality.

    $ ./tools/build.py project=iguana machine=<target> test_libs=all

### OK Linux

Before you begin, you should untar the OK Linux tarball into a subdirectory of your primary OKL4 tree called linux/.

OK Linux can be built using the following command:

    $ ./tools/build.py project=iguana machine=<target> wombat=yes

### Compiling for IA32 Targets

If the distributed toolchain binaries are used, add this argument to the command line:

    $ ./tools/build.py [options] toolprefix=i686-unknown-linux-gnu-

### How to 'make clean'

The equivalent of doing a `make clean` is to do:

    $ rm -rf build


Getting Around the Source
-------------------------

pistachio/          The OKL4 kernel

iguana/server       The OKL4 root server
iguana/example/     Sample OKL4 applications
iguana/test/        Test suite for the OKL4 root server
iguana/v*           Virtual device servers

drivers/            Device drivers

libs/               Core OKL4 libraries
libs/c              The OKL4 C library
libs/posix          A subset of the POSIX PSE51 profile
libs/l4[e]          Userland stubs for invoking kernel primitives
libs/iguana         OKL4 interface library for interacting with the OKL4 root server
libs/v*             OKL4 interface library for obtaining virtual devices

l4test/             Test suite for the OKL4 kernel

arch/               Architecture-specific code

platform/           Platform-specific code

projects/           Build-system configuration for projects

tools/              Build-system tools, IDL generator (magpie) and system-configurator (elfweaver)



Getting Help
------------

The latest public release of OKL4, along with relevant documentation, can be found at the [OKL4 Developer Wiki][6].

[6]: http://wiki.ok-labs.com/

There is an active and responsive [mailing list][7] for the OKL4 community on [developer@okl4.org][8].  You can subscribe by visiting [this page][7].

[7]: https://lists.okl4.org/mailman/listinfo/developer
[8]: mailto:developers@okl4.org



The Open Kernel Labs team
