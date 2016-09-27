#ifndef __UI_H__
#define __UI_H__

#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Con.h>

Ecore_Thread *thread;
Ecore_Timer *timer;
Evas_Object *combobox_dest;
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
void update_combobox_storage(Evas_Object *combobox);

static distro_t distrbibutions[];

#endif
