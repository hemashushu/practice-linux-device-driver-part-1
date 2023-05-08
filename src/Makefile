ifneq ($(KERNELRELEASE),)

obj-m += charone.o
obj-m += charone-counter.o
obj-m += charone-atomic.o
obj-m += charone-alloc-dev.o
obj-m += charone-multi-dev.o

else

# about the assign operator:
# - A = B
#   the A is an alias for B
# - A := B
#   the A is the value of B, e.g. A := $(shell pwd), the A will store the value of command
# - A ?= B
#   the A only update when it is not assigned before.

PWD := $(shell pwd)

# when intended to run on host machine
# KERNEL_SOURCE_DIR ?= /lib/modules/$(shell uname -r)/build

# when intended to run on the QEMU machine
KERNEL_SOURCE_DIR ?= /home/yang/projects/practice/practice-linux-device-driver-part-1/qemu/linux-6.2.10

all: user
	make -C $(KERNEL_SOURCE_DIR) M=$(PWD) modules

.PHONY: clean
clean:
	make -C $(KERNEL_SOURCE_DIR) M=$(PWD) clean
	test ! -f user-read || rm user-read

.PHONY: user
user:
	gcc -Wall -g -static -o user-read user-read.c
endif
