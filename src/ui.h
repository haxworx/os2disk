#ifndef __UI_H__
#define __UI_H__

#include <Elementary.h>
#include <Ecore.h>

Evas_Object *sha256_label;
Evas_Object *progressbar;
Evas_Object *bt_ok;
char *sha256sum;

typedef struct distro_t distro_t;
struct distro_t {
    char *name;
    char *url;
};

static char *remote_url = NULL;
static char *local_url = NULL;

void elm_window_create();

static distro_t distrbibutions[];

#endif
