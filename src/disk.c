#include "disk.h"

void system_get_disks(void)
{
#if defined(__OpenBSD__)
    static const mib[] = { CTL_HW, HW_DISKNAMES };
    static const unsigned int miblen = 2;
    char *drives;
    size_t len;

    sysctl(mib, miblen, NULL, &len, NULL, 0);

    if (!len) return;

    drives = calloc(1, len + 1);

    sysctl(mib, miblen, drives, &len, NULL, 0);

    int disk_count = 0;
    char buf[128];
    char *s = drives;
    while (s) {
        char *end = strchr(s, ':');
        *end = '\0';
        end++;
        if (s[0] == ',') s++;
        if (!s) break;
        if (!strcmp(s, "sd0")) goto skip;
        if (!strcmp(s, "hd0")) goto skip;
        if (!strncmp(s, "cd", 2)) goto skip;
        snprintf(buf, sizeof(buf), "/dev/%sc", s);
        storage[disk_count] = strdup(buf);
        disk_count++;
skip:
        s = strchr(end, ',');
    }
   
    storage[disk_count] = NULL;
#else 
    storage[0] = strdup("/dev/mmcblk1");
    storage[1] = NULL;
#endif   
}
