#ifndef __UI_H__
#define __UI_H__

#include <Elementary.h>
#include <Ecore.h>
#include <Ecore_Con.h>

static char *remote_url = NULL;
static char *local_url = NULL;

Ecore_Thread *thread;
Ecore_Timer *timer;

typedef struct Win_Main_Widgets Win_Main_Widgets;
struct Win_Main_Widgets { 
    Evas_Object *win;
    Evas_Object *icon;
    Evas_Object *box;
    Evas_Object *combobox_source;
    Evas_Object *combobox_dest;
    Evas_Object *sha256_label;
    Evas_Object *progressbar;
    Evas_Object *table;
    Evas_Object *bt_ok;
    Evas_Object *bt_cancel;
    char *sha256sum;
};

Win_Main_Widgets *window;

typedef struct distro_t distro_t;
struct distro_t {
    char *name;
    char *url;
};

Win_Main_Widgets *elm_window_create(void);
void update_combobox_storage(Evas_Object *combobox);

static distro_t distrbibutions[];

#endif
