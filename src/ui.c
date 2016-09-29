#include "ui.h"
#include "core.h"
#include "disk.h"

extern Ui_Main_Contents *ui;

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
    char buf[128];
    int i = (int)(uintptr_t)data;
    snprintf(buf, sizeof(buf), "%s", distributions[i]->name);
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


static Eina_Bool
gl_filter_get(void *data, Evas_Object *obj EINA_UNUSED, void *key)
{
    if (strlen((char *) key)) return EINA_TRUE;

    return EINA_TRUE;
}

void 
update_combobox_source(void)
{
    int i;

    Elm_Genlist_Item_Class *itc;
    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_get;
    
    for (i = 0; distributions[i] != NULL; i++) {
        elm_genlist_item_append(ui->combobox_source, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);
    }

    if (i) {
          elm_object_part_text_set(ui->combobox_source, "guide", "source...");
          elm_genlist_realized_items_update(ui->combobox_source);
    }
}

void
update_combobox_storage(void)
{
    int i;
    Elm_Genlist_Item_Class *itc;
    itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_dest_get;
    
    elm_genlist_clear(ui->combobox_dest);
     
    for (i = 0; storage[i] != NULL; i++) {
        elm_genlist_item_append(ui->combobox_dest, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);
        Elm_Object_Item *item = elm_genlist_first_item_get(ui->combobox_dest);
	elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
    } 

    if (i) {
        elm_object_part_text_set(ui->combobox_dest, "guide", "destination...");
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

    snprintf(buf, sizeof(buf), "%s", distributions[i]->url);

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
    ui->sha256sum = www_file_save(thread, remote_url, local_url);

    if (ecore_thread_check(thread)) {
       return;
    }
}

static void
thread_end(void *data, Ecore_Thread *thread)
{
    if (ui->sha256sum) {
        elm_object_text_set(ui->sha256_label, ui->sha256sum);
        free(ui->sha256sum);
    }

    elm_object_disabled_set(ui->bt_ok, EINA_FALSE);
    elm_progressbar_pulse(ui->progressbar, EINA_FALSE);
    while((ecore_thread_wait(thread, 0.1)) != EINA_TRUE);
}

static void
thread_feedback(void *data, Ecore_Thread *thread, void *msg)
{
    int *c = msg;

    elm_progressbar_value_set(ui->progressbar, (double) *c / 10000);

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
_bt_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   if (!remote_url) return;
   if (!local_url) return;

  printf("local: %s and remote: %s\n\n", remote_url, local_url);
   ecore_www_file_save(remote_url, local_url);

   return; 
    

   elm_object_disabled_set(ui->bt_ok, EINA_TRUE);
   elm_progressbar_pulse(ui->progressbar, EINA_TRUE);

   thread = ecore_thread_feedback_run(thread_do, thread_feedback, thread_end, thread_cancel,
                                        NULL, EINA_FALSE);
}


Ui_Main_Contents *elm_window_create(void)
{
    int i;
    ui = malloc(sizeof(Ui_Main_Contents));

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    ui->win = elm_win_util_standard_add("os2drive", "OS2disk");

    ui->icon = evas_object_image_add(evas_object_evas_get(ui->win));
    evas_object_image_file_set(ui->icon, "images/icon.ico", NULL);
    elm_win_icon_object_set(ui->win, ui->icon);
    evas_object_show(ui->icon);

    ui->box = elm_box_add(ui->win);
    evas_object_size_hint_weight_set(ui->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ui->win, ui->box);
    evas_object_show(ui->box);

    ui->combobox_source = elm_combobox_add(ui->win);
    evas_object_size_hint_weight_set(ui->combobox_source, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(ui->combobox_source, EVAS_HINT_FILL, 0);
    //elm_object_part_text_set(ui->combobox_source, "guide", "source...");
    elm_box_pack_end(ui->box, ui->combobox_source);

    evas_object_smart_callback_add(ui->combobox_source, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);
    evas_object_show(ui->combobox_source);

    ui->combobox_dest = elm_combobox_add(ui->win);
    evas_object_size_hint_weight_set(ui->combobox_dest, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(ui->combobox_dest, EVAS_HINT_FILL, 0);
    elm_object_part_text_set(ui->combobox_dest, "guide", "destination...");
    elm_box_pack_end(ui->box, ui->combobox_dest);
    evas_object_show(ui->combobox_dest);


    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "default";
    itc->func.text_get = gl_text_dest_get;

    for (i = 0; storage[i] != NULL; i++)
        elm_genlist_item_append(ui->combobox_dest, itc, (void *) (uintptr_t) i,
                NULL, ELM_GENLIST_ITEM_NONE, NULL, (void *)(uintptr_t) i);

    evas_object_smart_callback_add(ui->combobox_dest, "item,pressed",
                                  _combobox2_item_pressed_cb, NULL);

    ui->progressbar= elm_progressbar_add(ui->win);
    evas_object_size_hint_align_set(ui->progressbar, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(ui->progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_progressbar_pulse_set(ui->progressbar, EINA_TRUE);
    elm_progressbar_span_size_set(ui->progressbar, 1.0);
    elm_progressbar_unit_format_set(ui->progressbar, "%1.2f%%");
    elm_box_pack_end(ui->box, ui->progressbar);
    evas_object_show(ui->progressbar);

    ui->sha256_label = elm_label_add(ui->win);
    elm_box_pack_end(ui->box, ui->sha256_label);
    evas_object_size_hint_align_set(ui->sha256_label, 0.5, EVAS_HINT_FILL);
    evas_object_show(ui->sha256_label);

    ui->table = elm_table_add(ui->win);
    elm_box_pack_end(ui->box, ui->table);
    evas_object_show(ui->table);

    ui->bt_ok = elm_button_add(ui->win);
    elm_object_text_set(ui->bt_ok, "Write");
    evas_object_show(ui->bt_ok);
    elm_table_pack(ui->table, ui->bt_ok, 0, 0, 1, 1);

    evas_object_smart_callback_add(ui->bt_ok, "clicked", _bt_clicked_cb, NULL);

    ui->bt_cancel = elm_button_add(ui->win);
    elm_object_text_set(ui->bt_cancel, "Cancel");
    evas_object_show(ui->bt_cancel);
    elm_table_pack(ui->table, ui->bt_cancel, 1, 0, 1, 1);

    evas_object_smart_callback_add(ui->bt_cancel, "clicked", _bt_cancel_clicked_cb, NULL);

    evas_object_resize(ui->win, 400,100);

    uid_t euid = geteuid();
    if (euid != 0) {
        error_popup(ui->win);
    }

    evas_object_show(ui->win);

    evas_object_smart_callback_add(ui->win, "delete,request", win_del, NULL);
 
    return ui;
}


