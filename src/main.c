#include <Elementary.h>
#include "core.h"

Evas_Object *sha256_label;
Evas_Object *progressbar;
Evas_Object *bt_ok;

typedef struct distro_t distro_t;
struct distro_t {
    char *name;
    char *url;
};

static char *remote_url = NULL;
static char *local_url = NULL;

distro_t distributions[] = {
    {"test", "http://enform.haxlab.org/files/default.edj"},
    {"Debian GNU/Linux v8.4 (i386/amd64)", "http://gensho.acc.umu.se/debian-cd/8.4.0/multi-arch/iso-cd/debian-8.4.0-amd64-i386-netinst.iso"},
    {"FreeBSD v10.3 (x86)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-i386-memstick.img"},
    {"FreeBSD v10.3 (amd64)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-amd64-memstick.img"},
    {"NetBSD v7.0 (i386)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-i386.iso"},
    {"NetBSD v7.0 (amd64)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-amd64.iso"},
    {"OpenBSD v6.0 (i386)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/i386/install60.fs"},
    {"OpenBSD v6.0 (amd64)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/amd64/install60.fs"},
    {NULL, NULL},
};

char *storage[] = {
    "custom file...",
    "/dev/sd1c",
    "/dev/null",
    "file.img",
    NULL,
};

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
    char buf[128];
    int i = (int)(uintptr_t)data;
    snprintf(buf, sizeof(buf), "%s", distributions[i].name);
    return strdup(buf);
}

static char *
gl_text_dest_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
    char buf[128];
    int i = (int)(uintptr_t)data;
    snprintf(buf, sizeof(buf), "%s", storage[i]);
    return strdup(buf);
}

static void
_combobox2_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'selected' callback is called. (selected item : %s)\n", txt);
}

static void
_combobox_item_pressed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                      void *event_info)
{
    char buf[256];
    const char *txt = elm_object_item_text_get(event_info);
    int i = (int)(uintptr_t) elm_object_item_data_get(event_info);

    snprintf(buf, sizeof(buf), "%s", distributions[i].url);

    if (remote_url) free(remote_url);
    remote_url = strdup(buf);

    elm_object_text_set(obj, txt);
    elm_combobox_hover_end(obj);
}

static void
_combobox2_item_pressed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                      void *event_info)
{
    char buf[256];
    int i = (int)(uintptr_t) elm_object_item_data_get(event_info);
    const char *txt = elm_object_item_text_get(event_info);
    snprintf(buf, sizeof(buf), "%s", storage[i]);

    if (local_url) free(local_url);
    local_url = strdup(txt);

    elm_object_text_set(obj, txt);
    elm_combobox_hover_end(obj);
}

char *sha256sum = NULL;

static void
thread_do(void *data, Ecore_Thread *thread)
{
    int count = 0;
    sha256sum = os_fetch_and_write(remote_url, local_url);

    if (ecore_thread_check(thread)) {
       return; 
    }
}

static void
thread_end(void *data, Ecore_Thread *thread)
{
    if (sha256sum) {
        elm_object_text_set(sha256_label,sha256sum); 
        printf("it is %s\n", sha256sum);
        free(sha256sum);
    }

    elm_object_disabled_set(bt_ok, EINA_FALSE);   
    elm_progressbar_pulse(progressbar, EINA_FALSE);
    while((ecore_thread_wait(thread, 0.1)) != EINA_TRUE);
}

static void
thread_feedback(void *data, Ecore_Thread *thread, void *msg)
{
    int *progress = msg;
    printf("progress is %d\n\n", *progress);
    free(progress);
}

#define thread_cancel thread_end

static void
del(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
    elm_exit();
}

static void
_bt_clicked_cb(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   (void) data;
   if (!remote_url) return;
   if (!local_url) return;

   elm_object_disabled_set(obj, EINA_TRUE);
   elm_progressbar_pulse(progressbar, EINA_TRUE);
   
   printf("remote: %s and local: %s\n", remote_url, local_url);
   thread = ecore_thread_feedback_run(thread_do, thread_feedback, thread_end, thread_cancel,
					NULL, EINA_FALSE);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    int i;

    ecore_init(); 
    elm_init(argc, argv);

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    Evas_Object *win = elm_win_util_standard_add("os2drive", "OS2disk");

    Evas_Object *box = elm_box_add(win);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, box);
    evas_object_show(box); 
    
    Evas_Object *combobox_source = elm_combobox_add(win);
    evas_object_size_hint_weight_set(combobox_source, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(combobox_source, EVAS_HINT_FILL, 0);
    elm_object_part_text_set(combobox_source, "guide", "image...");
    elm_box_pack_end(box, combobox_source);
    evas_object_show(combobox_source);

    Elm_Genlist_Item_Class *itc;
    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_get;

    for (i = 0; distributions[i].name != NULL; i++)
        elm_genlist_item_append(combobox_source, itc, (void *) (uintptr_t) i,
		NULL, ELM_GENLIST_ITEM_NONE, NULL,
                (void *) (uintptr_t) i);

    evas_object_smart_callback_add(combobox_source, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);


    Evas_Object *combobox_dest = elm_combobox_add(win);
    evas_object_size_hint_weight_set(combobox_dest, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(combobox_dest, EVAS_HINT_FILL, 0);
    elm_object_part_text_set(combobox_dest, "guide", "destination...");
    elm_box_pack_end(box, combobox_dest);
    evas_object_show(combobox_dest);


    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_dest_get;

    for (i = 0; storage[i] != NULL; i++)
        elm_genlist_item_append(combobox_dest, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);
     
    evas_object_smart_callback_add(combobox_dest, "item,pressed",
                                  _combobox2_item_pressed_cb, NULL);

    progressbar= elm_progressbar_add(win);
    evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_progressbar_pulse_set(progressbar, EINA_TRUE);
    elm_progressbar_span_size_set(progressbar, 1.0);
    elm_box_pack_end(box, progressbar);
    evas_object_show(progressbar);

    sha256_label = elm_label_add(win);
    elm_box_pack_end(box, sha256_label);
    evas_object_size_hint_align_set(sha256_label, 0.5, EVAS_HINT_FILL);
    evas_object_show(sha256_label);

    Evas_Object *table = elm_table_add(win);
    elm_box_pack_end(box, table);
    evas_object_show(table);

    bt_ok = elm_button_add(win);
    elm_object_text_set(bt_ok, "Write");
    evas_object_show(bt_ok);
    elm_table_pack(table, bt_ok, 0, 0, 1, 1);

    evas_object_smart_callback_add(bt_ok, "clicked", _bt_clicked_cb, NULL);

    Evas_Object *bt_about = elm_button_add(win);
    elm_object_text_set(bt_about, "About");
    evas_object_show(bt_about);
    elm_table_pack(table, bt_about, 1, 0, 1, 1);

    Evas_Object *bt_cancel = elm_button_add(win);
    elm_object_text_set(bt_cancel, "Cancel");
    evas_object_show(bt_cancel);
    elm_table_pack(table, bt_cancel, 2, 0, 1, 1);

    evas_object_resize(win, 400,100);
    evas_object_show(win);
    
    evas_object_smart_callback_add(win, "delete,request", del, NULL);
   
    elm_run();
   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

