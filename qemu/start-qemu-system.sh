#!/bin/bash
qemu-system-x86_64 \
    -machine q35 \
    -m 1G \
    -kernel ./linux-6.2.10/arch/x86/boot/bzImage \
    -append "root=/dev/vda1 rw console=ttyS0" \
    -drive file=vda.img,format=raw,if=virtio \
    -fsdev local,id=sf,path=/home/yang/projects/practice/practice-linux-device-driver-part-1/src/,security_model=mapped-file \
    -device virtio-9p-pci,fsdev=sf,mount_tag=hostshare \
    -nographic
