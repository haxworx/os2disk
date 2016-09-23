#ifndef __DISK_H__
#define __DISK_H__
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DISKS 10
char *storage[MAX_DISKS];

void system_get_disks(void);

#endif
