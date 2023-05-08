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

static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

#define DEVICE_NAME "charone" /* Dev name as it appears in /proc/devices   */

// note:
// A global static variable is one that can only be accessed in the file where it is created.
// This variable is said to have file scope.

static int major;         /* major number, which is assigned to our device driver automatically by system */
static struct class *cls; /* used by device_create(), the function then create the device file `/dev/charone` */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
};

const char CHAR_SEND = '1';

static int __init startup(void)
{
    // register driver with dynamatical major number
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0)
    {
        pr_alert("register device '%s' failed: %d\n", DEVICE_NAME, major);
        return major;
    }

    pr_info("succeed to register device: %s, major number is: %d\n", DEVICE_NAME, major);

    // create device file `/dev/charone`
    cls = class_create(THIS_MODULE, DEVICE_NAME);

    // device_create(
    //     struct class *cls, struct device *parent, dev_t devt,
    //     void *drvdata, const char *fmt, ...);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("device file created: /dev/%s\n", DEVICE_NAME);

    return 0; // SUCCESS
}

static void __exit cleanup(void)
{
    device_destroy(cls, MKDEV(major, 0)); // delete device file `/dev/charone`
    class_destroy(cls);

    pr_info("device file removed: /dev/%s\n", DEVICE_NAME);

    // unregister the driver
    unregister_chrdev(major, DEVICE_NAME);

    pr_info("succeed to unregister device: %s\n", DEVICE_NAME);
}

/* Called when a process, which already opened the dev file, attempts to
 * read from it.
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

/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    pr_info("/dev/%s: write - operation is not supported.\n", DEVICE_NAME);
    return -EINVAL;
}

module_init(startup);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hippo Spark");
MODULE_DESCRIPTION("A char driver for print infinite char `1`");