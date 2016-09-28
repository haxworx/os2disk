#include "disk.h"
#include "ui.h"

void _clear_storage(void)
{
    int i;

    for (i = 0; i < MAX_DISKS; i++) {
       if (storage[i]) {
          free(storage[i]);
          storage[i] = NULL;
       } 
    }
}

int system_get_disks(void)
{
    int disk_count = 0;

    _clear_storage();

#if defined(__OpenBSD__)
    static const mib[] = { CTL_HW, HW_DISKNAMES };
    static const unsigned int miblen = 2;
    char *drives;
    size_t len;

    sysctl(mib, miblen, NULL, &len, NULL, 0);

    if (!len) return;

    drives = calloc(1, len + 1);

    sysctl(mib, miblen, drives, &len, NULL, 0);

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
        printf("buffer: %s\n\n", buf);
        storage[disk_count++] = strdup(buf);
skip:
        s = strchr(end, ',');
    }
#else 
    eeze_init();
    Eina_List *devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
    Eina_List *l;
    char *data;

    EINA_LIST_FOREACH(devices, l, data) {
        char buf[PATH_MAX];
      
	snprintf(buf, sizeof(buf), "%s/block", data);

	DIR *dir = opendir(buf);
	struct dirent *dh;
	while (dh = readdir(dir)) {
            if (dh->d_name[0] == '.') continue;
	    char p[PATH_MAX];
            snprintf(p, sizeof(p), "%s/%s", buf, dh->d_name);
            struct stat fstats;
	    stat(p, &fstats);
	    if (S_ISDIR(fstats.st_mode)) {
                char devpath[PATH_MAX];
                snprintf(devpath, sizeof(devpath), "/dev/%s", dh->d_name);
	        storage[disk_count++] = strdup(devpath);
		break;
	    }
	}
	closedir(dir); 
    }

    eina_list_free(devices);

    eeze_shutdown();
#endif   
    storage[disk_count] = NULL;

    if (disk_count) {
        if (window) {
            update_combobox_storage(window->combobox_dest);
        }
    }
 
    return disk_count;
}

