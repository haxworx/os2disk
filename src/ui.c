#include "ui.h"
#include "core.h"
#include "disk.h"

static distro_t distributions[] = {
    {"Debian GNU/Linux v8.6 (i386/amd64)", "http://debian.inode.at/debian-cd/8.6.0/multi-arch/iso-cd/debian-8.6.0-amd64-i386-netinst.iso"},
    {"FreeBSD v10.3 (x86)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-i386-memstick.img"},
    {"FreeBSD v10.3 (amd64)", "http://ftp.freebsd.org/pub/FreeBSD/releases/ISO-IMAGES/10.3/FreeBSD-10.3-RELEASE-amd64-memstick.img"},
    {"NetBSD v7.0 (i386)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-i386.iso"},
    {"NetBSD v7.0 (amd64)", "http://mirror.planetunix.net/pub/NetBSD/iso/7.0/NetBSD-7.0-amd64.iso"},
    {"OpenBSD v6.0 (i386)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/i386/install60.fs"},
    {"OpenBSD v6.0 (amd64)", "http://mirror.ox.ac.uk/pub/OpenBSD/6.0/amd64/install60.fs"},
    {NULL, NULL},
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

void
update_combobox_storage(Evas_Object *combobox)
{
    int i;

    Elm_Genlist_Item_Class *itc;
    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_dest_get;
    
    elm_genlist_clear(combobox);
     
    for (i = 0; storage[i] != NULL; i++) {
        elm_genlist_item_append(combobox, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);
        Elm_Object_Item *item = elm_genlist_first_item_get(combobox);
	elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
    } 

    if (i) {
        elm_object_part_text_set(window->combobox_dest, "guide", "destination...");
    }
}

void error_popup(Evas_Object *win)
{
    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    elm_win_autodel_set(win, EINA_TRUE);

    Evas_Object *content = elm_label_add(win);
    elm_object_text_set(content, "<align=center>You don't have valid permissions. <br>Try running with 'sudo' or as root.</align>");

    Evas_Object *popup = elm_popup_add(win);

    elm_object_content_set(popup, content);

    elm_object_part_text_set(popup, "title,text", "Error");
   
    evas_object_show(popup);

    evas_object_smart_callback_add(popup, "block,clicked", NULL, NULL);

    evas_object_show(win);

    elm_run();

    exit(1); 
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

static void
thread_do(void *data, Ecore_Thread *thread)
{
    int count = 0;
    window->sha256sum = os_fetch_and_write(thread, remote_url, local_url);

    if (ecore_thread_check(thread)) {
       return;
    }
}

static void
thread_end(void *data, Ecore_Thread *thread)
{
    if (window->sha256sum) {
        elm_object_text_set(window->sha256_label, window->sha256sum);
        free(window->sha256sum);
    }

    elm_object_disabled_set(window->bt_ok, EINA_FALSE);
    elm_progressbar_pulse(window->progressbar, EINA_FALSE);
    while((ecore_thread_wait(thread, 0.1)) != EINA_TRUE);
}

static void
thread_feedback(void *data, Ecore_Thread *thread, void *msg)
{
    int *c = msg;

    elm_progressbar_value_set(window->progressbar, (double) *c / 10000);

    free(c);
}

#define thread_cancel thread_end

static void
win_del(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
    if (timer) {
        ecore_timer_del(timer);
    }
    ecore_main_loop_quit();
    elm_exit();
}

static void
_bt_cancel_clicked_cb(void *data, Evas_Object *obj, void *event)
{
    evas_object_del(obj);

    if (timer) {
        ecore_timer_del(timer);
    }
    
    while((ecore_thread_wait(thread, 0.1)) != EINA_TRUE);

    elm_exit();
}

static void
_bt_clicked_cb(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   (void) data;
   if (!remote_url) return;
   if (!local_url) return;

   /*
   ecore_os_fetch_and_write(remote_url, local_url);

   return; 
   */

   elm_object_disabled_set(window->bt_ok, EINA_TRUE);
   elm_progressbar_pulse(window->progressbar, EINA_TRUE);

   printf("remote: %s and local: %s\n", remote_url, local_url);
   thread = ecore_thread_feedback_run(thread_do, thread_feedback, thread_end, thread_cancel,
                                        NULL, EINA_FALSE);
}


Win_Main_Widgets *elm_window_create(void)
{
    int i;
    window = malloc(sizeof(Win_Main_Widgets));

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    window->win = elm_win_util_standard_add("os2drive", "OS2disk");

    window->icon = evas_object_image_add(evas_object_evas_get(window->win));
    evas_object_image_file_set(window->icon, "images/icon.ico", NULL);
    elm_win_icon_object_set(window->win, window->icon);
    evas_object_show(window->icon);

    window->box = elm_box_add(window->win);
    evas_object_size_hint_weight_set(window->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(window->win, window->box);
    evas_object_show(window->box);

    window->combobox_source = elm_combobox_add(window->win);
    evas_object_size_hint_weight_set(window->combobox_source, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(window->combobox_source, EVAS_HINT_FILL, 0);
    elm_object_part_text_set(window->combobox_source, "guide", "source...");
    elm_box_pack_end(window->box, window->combobox_source);

    Elm_Genlist_Item_Class *itc;
    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_get;

    for (i = 0; distributions[i].name != NULL; i++)
        elm_genlist_item_append(window->combobox_source, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL,
                (void *) (uintptr_t) i);

    evas_object_smart_callback_add(window->combobox_source, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);

    Elm_Object_Item *item = elm_genlist_first_item_get(window->combobox_source);
    elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
    elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
    evas_object_show(window->combobox_source);

    window->combobox_dest = elm_combobox_add(window->win);
    evas_object_size_hint_weight_set(window->combobox_dest, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(window->combobox_dest, EVAS_HINT_FILL, 0);
    elm_object_part_text_set(window->combobox_dest, "guide", "destination...");
    elm_box_pack_end(window->box, window->combobox_dest);
    evas_object_show(window->combobox_dest);


    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_dest_get;

    for (i = 0; storage[i] != NULL; i++)
        elm_genlist_item_append(window->combobox_dest, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);

    evas_object_smart_callback_add(window->combobox_dest, "item,pressed",
                                  _combobox2_item_pressed_cb, NULL);
    window->progressbar= elm_progressbar_add(window->win);
    evas_object_size_hint_align_set(window->progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(window->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_progressbar_pulse_set(window->progressbar, EINA_TRUE);
    elm_progressbar_span_size_set(window->progressbar, 1.0);
    elm_progressbar_unit_format_set(window->progressbar, "%1.2f%%");
    elm_box_pack_end(window->box, window->progressbar);
    evas_object_show(window->progressbar);

    window->sha256_label = elm_label_add(window->win);
    elm_box_pack_end(window->box, window->sha256_label);
    evas_object_size_hint_align_set(window->sha256_label, 0.5, EVAS_HINT_FILL);
    evas_object_show(window->sha256_label);

    window->table = elm_table_add(window->win);
    elm_box_pack_end(window->box, window->table);
    evas_object_show(window->table);

    window->bt_ok = elm_button_add(window->win);
    elm_object_text_set(window->bt_ok, "Write");
    evas_object_show(window->bt_ok);
    elm_table_pack(window->table, window->bt_ok, 0, 0, 1, 1);

    evas_object_smart_callback_add(window->bt_ok, "clicked", _bt_clicked_cb, NULL);

    window->bt_cancel = elm_button_add(window->win);
    elm_object_text_set(window->bt_cancel, "Cancel");
    evas_object_show(window->bt_cancel);
    elm_table_pack(window->table, window->bt_cancel, 1, 0, 1, 1);

    evas_object_smart_callback_add(window->bt_cancel, "clicked", _bt_cancel_clicked_cb, NULL);

    evas_object_resize(window->win, 400,100);

    uid_t euid = geteuid();
    if (euid != 0) {
        error_popup(window->win);
    }

    evas_object_show(window->win);

    evas_object_smart_callback_add(window->win, "delete,request", win_del, NULL);
 
    return window;
}


