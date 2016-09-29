#include "ui.h"
#include "core.h"

Ui_Main_Contents *ui = NULL;

char buffer[65535];
ssize_t total = 0;

void 
populate_list(void)
{
    char *start = &buffer[0];
    int count = 0;

    while (start) {
        char *end = strchr(start, '=');
        if (!end) break;
        *end = '\0'; 
        char *name = strdup(start);
        start = end + 1; end++;
        while (end[0] != '\n') {
            end++;
	}
        *end = '\0';
        char *url = strdup(start);
        printf("url %s\n", url);
        printf("name %s\n\n", name); 
        distributions[count] = malloc(sizeof(distro_t));
        distributions[count]->name = strdup(name);
        distributions[count++]->url = strdup(url); 

        start = end + 1;
    } 

    distributions[count] = NULL;
    update_combobox_source();
}

static Eina_Bool
_data_cb(void *data, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Event_Url_Data *url_data = event_info;
    char *buf = &buffer[total];

    int i;

    for (i = 0; i < url_data->size; i++) {
        buf[i] = url_data->data[i];
    } 
    total += url_data->size; 
}

static Eina_Bool
_download_complete_cb(void *data, int type EINA_UNUSED, void *event_info)
{
    Ecore_Con_Event_Url_Complete *url_complete = event_info;
    Ecore_Con_Url *h = data;
    printf("STATUS %d\n\n",  url_complete->status);
    populate_list();

    ecore_con_url_free(h);
}

Eina_Bool
get_distribution_list()
{
    if (!ecore_con_url_pipeline_get()) {
        ecore_con_url_pipeline_set(EINA_TRUE);
    }

    Ecore_Con_Url *h = ecore_con_url_new("http://haxlab.org/list.txt");

    ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _data_cb, NULL);
    ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _download_complete_cb, h);

    ecore_con_url_get(h);
}

Eina_Bool
system_check_changes(void *data)
{
    (void) data;
    int count = system_get_disks();
    if (count) {
        ecore_timer_del(timer);
        timer = NULL;
    }

    return ECORE_CALLBACK_RENEW;
}


EAPI_MAIN int
elm_main(int argc, char **argv)
{
    ecore_init(); 
    ecore_con_init();
    ecore_con_url_init();

    elm_init(argc, argv);

    if (!system_get_disks()) {
        /* keep checking until disk found */ 
        timer = ecore_timer_add(3.0, system_check_changes, NULL);
    }

    get_distribution_list();

    ui = elm_window_create();

    ecore_main_loop_begin();
    
    ecore_con_url_shutdown();
    ecore_con_shutdown();   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

