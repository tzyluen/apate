Apate
=====
A trivial linux module keylogger for educational purposes.

Files:
-----
* apate.h - apate keylogger header file
* apate.c - apate keylogger module file
* deity.c - tester to read logged keys from the char device file
* create_dev.sh - create the device in /dev

Quick Guide:
-----------
```
0. $ sudo mknod -m a+r+w /dev/apate c 78 1
1. $ make
2. $ sudo insmod apate.ko
3. $ cat /dev/apate
OR
4. $ gcc deity.c -o deity
5. $ ./deity
6. $ sudo rmmod apate.ko
```

Howto:
-----
```
* Create the device in /dev/:
1. udev methods:
    1. $ touch /etc/udev/rules.d/10-local.rules
    2. insert,
       KERNEL=="apate", NAME="apate", OWNER="tzy", MODE="0666"
    3. $ sudo /sbin/udevadm control --reload-rules
    4. $ sudo /sbin/udevadm trigger

2. manual, mknod methods:
   1. $ mknod -m a+r+w /dev/apate c 78 1
```
