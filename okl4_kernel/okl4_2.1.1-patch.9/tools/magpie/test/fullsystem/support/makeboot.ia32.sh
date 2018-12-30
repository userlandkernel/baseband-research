#!/bin/sh

# This creates a floppy image. It might be better to make a CD image as per
# these instructions on the l4 hax0rs list:
#   1.  create a directory which will contain all the files of the
#   iso-image. Lets call it iso
#   2. in the iso directory create a boot/grub directory
#   2b. put the file stage2_eltorito into that dir (I found it on my Debian
#   distro here: /usr/lib/grub/i386-pc/; you can also get our modified grub
#   from SVN and build the stage2 file yourself)
#   NFD: May need modified grub here: http://os.inf.tu-dresden.de/~adam/grub/
#   3. put the files you want to boot somewhere into the iso directory
#(preferably into boot/)
#   4. create the respective Grub entries in boot/grub/menu.lst (have a
#look at the demo-cd boot/grub/menu.lst for examples)
#   5. mkisofs -R -b boot/grub/stage2_eltorito \
#        -no-emul-boot -boot-load-size 4 -boot-info-table \
#        -hide-rr-moved \
#        -J -joliet-long \
#        -o "$isoimage" .
#   6. boot the image with 'qemu -serial stdio -cdrom image'

GRUB=/usr/sbin/grub
GRUB1=/usr/lib/grub/i386-pc/stage1
GRUB2=/usr/lib/grub/i386-pc/stage2

# Use mtools to make a hard disk image for qemu.
# Based on code at: http://l4ka.org/projects/pistachio/ia32/gettingstarted.php

#dd if=/dev/zero of=build/fdimage.img bs=512 count=2880
dd if=/dev/zero of=build/fdimage.img bs=512 count=5760

echo 'drive a: file="build/fdimage.img"' > build/mtoolsrc
MTOOLSRC=./build/mtoolsrc mformat -f 2880 a:
MTOOLSRC=./build/mtoolsrc mmd a:/boot
MTOOLSRC=./build/mtoolsrc mmd a:/boot/grub
MTOOLSRC=./build/mtoolsrc mcopy $GRUB1 a:/boot/grub
MTOOLSRC=./build/mtoolsrc mcopy $GRUB2 a:/boot/grub
MTOOLSRC=./build/mtoolsrc mcopy support/grub.conf a:/boot/grub/grub.cnf
MTOOLSRC=./build/mtoolsrc mcopy build/kernel/ia32-kernel a:/kernel
MTOOLSRC=./build/mtoolsrc mcopy build/user/util/kickstart/kickstart a:/kstart
MTOOLSRC=./build/mtoolsrc mcopy build/user/serv/sigma0/sigma0 a:/
#MTOOLSRC=./build/mtoolsrc mcopy build/user/apps/bench/pingpong/pingpong a:/
MTOOLSRC=./build/mtoolsrc mcopy build/user/apps/nest/nest a:/

echo "(fd0) build/fdimage.img" > build/bmap
cat <<EOF | $GRUB --batch --device-map=build/bmap
root (fd0)
install /boot/grub/stage1 (fd0) /boot/grub/stage2 /boot/grub/grub.cnf
quit
EOF

echo Run with qemu -nographic -fda build/fdimage.img



