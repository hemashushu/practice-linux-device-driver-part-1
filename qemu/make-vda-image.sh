#!/bin/bash
set -ex

## method 1
#
# test ! -f vda.img || rm vda.img
# dd if=/dev/zero of=vda.img bs=1M count=128
# mkfs.ext2 -F vda.img
# mkdir -p mnt
# sudo mount -o loop vda.img mnt

## method 2
#
# $ dd if=/dev/zero of=vda.img bs=1M count=128
# $ fdisk vda.img
# (fdisk) g // create a new empty GPT partition table
# (fdisk) n // add a new partition, all default
# (fdisk) w // write table to disk and exit
#
# $ fdisk -l vda.img
#
# Disk vda.img: 128 MiB, 134217728 bytes, 262144 sectors
# Sector size (logical/physical): 512 bytes / 512 bytes
# Disklabel type: gpt
# Device     Start    End Sectors  Size Type
# vda.img1    2048 260095  258048  126M Linux filesystem

sudo losetup -P /dev/loop0 vda.img
# sudo mkfs.ext2 /dev/loop0p1
mkdir -p mnt
sudo mount /dev/loop0p1 mnt

cd mnt

sudo mkdir -p bin sbin lib usr/bin usr/sbin usr/lib 
sudo mkdir -p etc opt
sudo mkdir -p dev proc sys run tmp
sudo mkdir -p root root/share
sudo cp ../busybox-1.36.0/busybox bin

pushd bin
test -L sh || sudo ln -s busybox sh
popd

pushd sbin
test -L init || sudo ln -s ../bin/busybox init
test -L mount || sudo ln -s ../bin/busybox mount
popd

# to resolve the busybox complaint on the first boot
pushd dev
# sudo mknod -m 666 null c 1 3
# sudo mknod -m 644 console c 5 1
# sudo mknod -m 666 tty c 5 0
# sudo mknod -m 666 tty0 c 4 0
# sudo mknod -m 666 tty1 c 4 0
# sudo mknod -m 660 ttyS0 c 4 64
sudo mknod -m 666 tty2 c 4 0
sudo mknod -m 666 tty3 c 4 0
sudo mknod -m 666 tty4 c 4 0
popd

pushd etc
sudo mkdir -p init.d

cat << EOF | sudo tee init.d/rcS
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t tmpfs none /run
mount -t tmpfs none /tmp
mount -t 9p -o trans=virtio hostshare /root/share
EOF

sudo chmod +x init.d/rcS

# to resolve the busybox complaints on every shutdown
sudo touch fstab
popd

cd ..

sudo umount mnt
rm -r mnt

sudo losetup -d /dev/loop0

# run `/bin/busybox --install -s` after the first boot.
