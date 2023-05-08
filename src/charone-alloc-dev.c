/**
 * Copyright (c) 2023 Hemashushu <hippospark@gmail.com>, All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// NOTE::
// append `/home/yang/QEMU-X86-64-DRIVER/linux-6.2.10/include` path to include path.
// VSCode:
//     1. run command: `C/C++: Edit configurations (UI)` (i.e. press Ctrl+Shift+P and then type `C Edit config UI`)
//     2. scroll to `Include path` and input `/home/yang/QEMU-X86-64-DRIVER/linux-6.2.10/include/**`
//
// ```
// ${workspaceFolder}/**
// /home/yang/QEMU-X86-64-DRIVER/linux-6.2.10/include/**
// ```

#include <linux/module.h> /* needed by all modules, `MODULE_LICENSE()`, `MODULE_AUTHOR()`, `MODULE_DESCRIPTION()` */
#include <linux/printk.h> /* for `pr_info()` */
#include <linux/init.h>   /* for Macro `module_init()` and `module_exit()` */
#include <linux/kernel.h> /* for `ARRAY_SIZE()`, `sprintf()` */

/* for module parameters */
#include <linux/moduleparam.h> /* for `module_param()` and `MODULE_PARM_DESC()` */
#include <linux/stat.h>        /* for the permission bits in the macro `module_param()` */

/* for register and unregister device */
#include <linux/fs.h>     /* for `struct file_operations` and `register_chrdev()` */
#include <linux/device.h> /* for `class_create()` and `device_create()` */

/* for file access */
#include <linux/types.h>   /* for `loff_t` */
#include <linux/uaccess.h> /* for `get_user()` and `put_user()` */

#include <asm/errno.h> /* for `EINVAL`, `E...` */

/* atomic operations */
#include <linux/atomic.h> /* for `atomic_t`, `atomic_cmpxchg()` and `atomic_set()` */

#include <linux/cdev.h> /* for `cdev_add()` */

static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);

#define DEVICE_NAME "charone" /* Dev name as it appears in /proc/devices   */

// note:
// A global static variable is one that can only be accessed in the file where it is created.
// This variable is said to have file scope.

static int major_number;         /* major number, which is assigned to our device driver automatically by system */
static struct class *cls; /* used by device_create(), the function then create the device file `/dev/charone` */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

const char CHAR_SEND = '1';

enum
{
    DEVICE_CLOSED = 0,
    DEVICE_OPENED = 1,
};

/* a locker which is used to prevent multiple access to device */
static atomic_t locker = ATOMIC_INIT(DEVICE_CLOSED);

static unsigned int total_number_of_devices = 1;
static struct cdev my_char_dev;

static int __init startup(void)
{
    // register driver with dynamatical major number

    // `register_chrdev()` is the old method before kernel 2.6, it will generate many minor numbers and `cdev` by default.
    //
    /*
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
    {
        pr_alert("register device '%s' failed: %d\n", DEVICE_NAME, major_number);
        return major_number;
    }
    */

    // `dev_t` is the number of device, which includes major number and minor number
    // - get major number:
    //     MAJOR(dev_t dev)
    // - get minor number:
    //     MINOR(dev_t dev)
    dev_t dev_number;

    int alloc_ret = -1;
    int add_ret = -1;

    // int alloc_chrdev_region (
    //      dev_t *dev, unsigned int firstminor, unsigned int count, const char *name);
    //
    // param: *dev
    //        it is an output parameter
    // param: name
    //        the device name will appear in `/proc/devices`
    // return:
    //         0 when succeed.
    //         -x when failed.
    alloc_ret = alloc_chrdev_region(&dev_number, 0, total_number_of_devices, DEVICE_NAME);
    if (alloc_ret)
    {
        pr_alert("failed to allocate char device region\n");
        goto err;
    }

    major_number = MAJOR(dev_number);
    pr_info("succeed to allocate device: %s, major number is: %d\n", DEVICE_NAME, major_number);

    cdev_init(&my_char_dev, &fops);
    my_char_dev.owner = THIS_MODULE;

    dev_number = MKDEV(major_number, 0);
    // int cdev_add (struct cdev * p, dev_t dev, unsigned count);
    add_ret = cdev_add(&my_char_dev, dev_number, total_number_of_devices);
    if (add_ret)
    {
        pr_alert("failed to add char device\n");
        goto err;
    }
    pr_info("char device add successfully, major: %d, minor: %d\n", MAJOR(dev_number), MINOR(dev_number));

    // create device file `/dev/charone`
    cls = class_create(THIS_MODULE, DEVICE_NAME);

    dev_number = MKDEV(major_number, 0);
    // struct device * device_create(
    //     struct class *cls, struct device *parent, dev_t devt,
    //     void *drvdata, const char *fmt, ...);
    device_create(cls, NULL, dev_number, NULL, DEVICE_NAME);

    pr_info("device file created: /dev/%s\n", DEVICE_NAME);

    return 0; // SUCCESS

err:
    if (add_ret == 0)
    {
        cdev_del(&my_char_dev);
    }

    if (alloc_ret == 0)
    {
        unregister_chrdev_region(dev_number, total_number_of_devices);
    }

    return -1; // FAILURE
}

static void __exit cleanup(void)
{

    dev_t dev_number;

    dev_number = MKDEV(major_number, 0);
    device_destroy(cls, dev_number); // delete device file `/dev/charone`
    pr_info("device file removed: /dev/%s\n", DEVICE_NAME);

    class_destroy(cls);

    // unregister the driver
    /*
    unregister_chrdev(major, DEVICE_NAME);
    */

    dev_number = MKDEV(major_number, 0);
    cdev_del(&my_char_dev);
    pr_info("char device delete successfully, major: %d, minor: %d\n", MAJOR(dev_number), MINOR(dev_number));

    // void unregister_chrdev_region(dev_t first, unsigned int count);
    //
    // param: first
    //        the first device number (if request several major number)
    unregister_chrdev_region(dev_number, total_number_of_devices);

    pr_info("succeed to unregister device: %s\n", DEVICE_NAME);
}

/** Called when a process, which already opened the dev file, attempts to
 * read from it.
 * e.g.
 * `head -c 10 /dev/charone`
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
                           char __user *buffer, /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t *offset)
{
    int bytes_read = 0;

    while (length)
    {
        put_user(CHAR_SEND, buffer++);
        length--;
        bytes_read++;
    }

    pr_info("/dev/%s: read - complete %d bytes.\n", DEVICE_NAME, bytes_read);

    return bytes_read;
}

/** Called when a process writes to dev file,
 * e.g.
 * echo "hi" > /dev/charone
 */
static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    pr_info("/dev/%s: write - operation is not supported.\n", DEVICE_NAME);
    return -EINVAL;
}

/**
 * Called when a process closes the device file.
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int total_open = 0;
    int ref_count = 0;

    // lock
    if (atomic_cmpxchg(&locker, DEVICE_CLOSED, DEVICE_OPENED))
    {
        return -EBUSY;
    }

    // increment the reference count of the current module
    try_module_get(THIS_MODULE);

    total_open++;
    ref_count = module_refcount(THIS_MODULE);

    pr_info("/dev/%s: open - total open %d, current ref count: %d.\n", DEVICE_NAME, total_open, ref_count);

    return 0; // SUCCESS
}

/**
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    int ref_count = 0;

    // release
    atomic_set(&locker, DEVICE_CLOSED);

    // decrement the reference count of current module.
    module_put(THIS_MODULE);

    ref_count = module_refcount(THIS_MODULE);

    pr_info("/dev/%s: release - current ref count: %d.\n", DEVICE_NAME, ref_count);

    return 0; // SUCCESS
}

module_init(startup);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hippo Spark");
MODULE_DESCRIPTION("A char driver for print infinite char `1`");