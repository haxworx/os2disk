#include "disk.h"
#include "ui.h"

extern Ui_Main_Contents *ui;

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
    char buf[128];
    char *drives;
    size_t len;

    sysctl(mib, miblen, NULL, &len, NULL, 0);

    if (!len) return;

    drives = calloc(1, len + 1);
    if (!drives) {
        return 0;
    }

    sysctl(mib, miblen, drives, &len, NULL, 0);

    char *s = drives;
    while (s) {
        char *end = strchr(s, ':');
        if (!end) { 
            break;
        }        
      
        *end = '\0';
 
        /* Do not expose common drives */       
        if (!strcmp(s, "sd0") || !strcmp(s, "hd0") ||
            !strncmp(s, "cd", 2)) {
            goto skip;
        }

        if (s[0] == ',') {
            s++;
        }

        snprintf(buf, sizeof(buf), "/dev/%sc", s);
        printf("buffer: %s\n\n", buf);
        storage[disk_count++] = strdup(buf);
skip:
        end++;
        s = strchr(end, ',');
        if (!s) {
            break;
        }
    }

    free(drives);

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
        if (ui) {
            update_combobox_storage();
        }
    }
 
    return disk_count;
}

