#include "ui.h"
#include "core.h"

Ui_Main_Contents *ui = NULL;

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

    //ui = elm_window_create();
    ecore_main_loop_begin();
    
    ecore_con_url_shutdown();
    ecore_con_shutdown();   
    ecore_shutdown();
    elm_shutdown();

    return (0);
}

ELM_MAIN()

